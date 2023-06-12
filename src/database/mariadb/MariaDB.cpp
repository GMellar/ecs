/*
 * MariaDB.cpp
 *
 *  Created on: 07.06.2023
 *      Author: Geoffrey Mellar <mellar@house-of-plasma.com>
 *
 * Copyright (C) 2023 Geoffrey Mellar <mellar@house-of-plasma.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ecs/database/mariadb/MariaDB.hpp>
#include <ecs/database/impl/MigratorImpl.hpp>

std::mutex ecs::db3::MariaDBConnection::libraryInitMutex;
bool ecs::db3::MariaDBConnection::libraryInit = false;

class MigratorImplMariaDb : public ecs::db3::MigratorImpl {
public:
	MigratorImplMariaDb(ecs::db3::DbConnection *connection) : MigratorImpl(connection) {

	}

	virtual ~MigratorImplMariaDb() {

	}

	virtual int initSchema() final {
		/* Table creation works always because we use conditional creation */
		connection->execute(
			"CREATE TABLE IF NOT EXISTS schema_info ("
				"name VARCHAR(100),"
				"value VARCHAR(100),"
				"CONSTRAINT schema_info_pk PRIMARY KEY (name)"
			");"
		);

		/* Initial schema insertion needs try catch because
		 * there could be an initialized schema. Zero is always the
		 * initial version number.
		 */
		try {
			connection->execute(
				"INSERT INTO schema_info VALUES('version', '0');"
			);
		}catch(std::exception &e){
			return 0;
		}

		return 0;
	}

	virtual int getSchemaVersion() final {
		ecs::db3::Statement::sharedPtr_T stmt;

		stmt = connection->prepare("SELECT value FROM schema_info WHERE name='version';");
		auto result = stmt->execute();
		auto row    = result.fetch();

		if(!row) throw std::runtime_error("Schema version not available");
		return std::stoi(row.at(0).cast_reference<std::string>(), nullptr, 10);
	}

	virtual bool setSchemaVersion(int version) {
		ecs::db3::Statement::sharedPtr_T stmt;

		stmt = connection->prepare("UPDATE schema_info SET value=? WHERE name='version';");
		stmt->bind(std::to_string(version));
		return stmt->execute();
	}

	bool doMigration ( ecs::db3::Migrator::Migration::ptr_T migration ) {
		connection->startTransation();

		try {
			if(!migration->upMigration(connection)) {
				connection->rollbackTransaction();
				return false;
			}
		}catch(std::exception &) {
			connection->rollbackTransaction();
			throw;
		}

		connection->commitTransaction();
		return true;
	}
};

ecs::db3::MariaDBConnection::ConnectionWrapper::ConnectionWrapper(
		const ConnectionParameters &parameters) : connectionTimout(5) {
	std::scoped_lock lock(connectionMutex);

	/* Get the thread id of the connection
	 * to check if the connection needs to be
	 * cloned when passed to another thread.
	 */
	threadId = std::this_thread::get_id();

	connection.reset(mysql_init(nullptr), [](MYSQL *connection){
		mysql_close(connection);
	});

	if(connection) {
		auto optionrc = mysql_options(connection.get(), mysql_option::MYSQL_OPT_CONNECT_TIMEOUT, &connectionTimout);

		auto rc = mysql_real_connect(connection.get(),
				parameters.getHostname().c_str(),
				parameters.getUser().c_str(),
				parameters.getPassword().c_str(),
				parameters.getDbName().c_str(),
				parameters.getPort(),
				nullptr,
				0);

		if(rc == nullptr) {
			errorMessage = mysql_error(connection.get());
			connection.reset();
		}
	}
}

ecs::db3::MariaDBConnection::ConnectionWrapper::~ConnectionWrapper() {

}

ecs::db3::MariaDBStatement::MariaDBStatement(
		const std::shared_ptr<MariaDBConnection::ConnectionWrapper> &connection,
		const std::string &query) : connection(connection), query(query) {
	std::scoped_lock lock(connection->connectionMutex);

	statement.reset(mysql_stmt_init(connection->connection.get()), [](MYSQL_STMT *stmt){
		mysql_stmt_close(stmt);
	});

	if(statement) {
		auto rc = mysql_stmt_prepare(statement.get(), query.c_str(), query.size());

		if(rc != 0) {
			setErrorString(mysql_stmt_error(statement.get()));
			statement.reset();
		}
	}

}

ecs::db3::MariaDBStatement::~MariaDBStatement() {
	metaResult.reset();
	statement.reset();
	connection.reset();
}

int ecs::db3::MariaDBStatement::execute(Table *table) {
	int rc;

	if(statement) {
		std::scoped_lock lock(connection->connectionMutex);

		rc = parameterBindings.bind(statement.get());
		if(rc != 0) {
			setErrorString(mysql_stmt_error(statement.get()));
			return -1;
		}

		/* Execute statement and handle errors */
		rc = mysql_stmt_execute(statement.get());

		/* In case f an error stop immediately and
		 * return the error string.
		 */
		if(rc != 0) {
			setErrorString(mysql_stmt_error(statement.get()));
			return -1;
		}

		/* Fetch the result metadata which is later used
		 * to fetch the column data.
		 */
		metaResult.reset(mysql_stmt_result_metadata(statement.get()), [](MYSQL_RES *meta){
			mysql_free_result(meta);
		});

		if(!metaResult) {
			return 0;
		}

		/* Fetch the column count of the result */
		auto resultColumnCount = mysql_num_fields(metaResult.get());
		auto fields            = mysql_fetch_fields(metaResult.get());

		resultBindings.resize(resultColumnCount);
		resultValues.clear();
		resultValues.reserve(resultColumnCount);
		std::memset(resultBindings.data(), '\0', resultBindings.size() * sizeof(MYSQL_BIND));

		/* Fetch the column names of the result table */
		for(unsigned int i = 0;i < resultColumnCount;++i) {
			//auto field = mysql_stmt_param_metadata()
			const MYSQL_FIELD * field = &fields[i];

			std::unique_ptr<BindHolder> value = std::make_unique<BindHolder>(&resultBindings[i]);

			/* Put field names into the result table */
			table->columnNames.push_back(std::string(field->name, field->name_length));

			resultBindings[i].buffer        = nullptr;
			resultBindings[i].length        = &value->length;
			resultBindings[i].is_null       = &value->isnull;
			resultBindings[i].buffer_length = 0;

			/* Get the types of the result columns */
			switch(field->type) {
				case MYSQL_TYPE_TINY:     /* 8 bit */
				case MYSQL_TYPE_SHORT:    /* 16 bit */
				case MYSQL_TYPE_LONG:     /* 32 bit */
				case MYSQL_TYPE_LONGLONG: /* 64 bit */
					resultBindings[i].buffer_type = MYSQL_TYPE_LONGLONG;
					if(field->flags & UNSIGNED_FLAG) {
						value->values      = std::uint64_t();
						resultBindings[i].buffer = (void*)&std::get<std::uint64_t>(value->values);
						value->cellFactory = [](BindHolder *bind){
							return ecs::tools::any::make_unique<ecs::db3::types::Uint64>(std::get<std::uint64_t>(bind->values));
						};
					}else{
						value->values      = std::int64_t();
						resultBindings[i].buffer = (void*)&std::get<std::int64_t>(value->values);
						value->cellFactory = [](BindHolder *bind){
							return ecs::tools::any::make_unique<ecs::db3::types::Int64>(std::get<std::int64_t>(bind->values));
						};
					}
					break;
				case MYSQL_TYPE_DOUBLE:
					value->values                   = double();
					resultBindings[i].buffer_type   = MYSQL_TYPE_DOUBLE;
					resultBindings[i].buffer        = (void*)&std::get<double>(value->values);
					value->cellFactory = [](BindHolder *bind){
						return ecs::tools::any::make_unique<ecs::db3::types::Double>(std::get<double>(bind->values));
					};
					break;
				case MYSQL_TYPE_FLOAT:
					value->values                   = float();
					resultBindings[i].buffer_type   = MYSQL_TYPE_FLOAT;
					resultBindings[i].buffer        = (void*)&std::get<float>(value->values);
					value->cellFactory = [](BindHolder *bind){
						return ecs::tools::any::make_unique<ecs::db3::types::Float>(std::get<float>(bind->values));
					};
					break;
				case MYSQL_TYPE_STRING:
				case MYSQL_TYPE_VAR_STRING:
				case MYSQL_TYPE_VARCHAR:
					value->values                   = std::vector<char>();
					resultBindings[i].buffer_type   = field->type;
					value->cellFactory = [](BindHolder *bind){
						std::vector<char> &container = std::get<std::vector<char>>(bind->values);
						auto result = ecs::tools::any::make_unique<ecs::db3::types::String>();
						result->cast<std::string>()->assign(container.cbegin(), container.cend());
						return result;
					};
					break;
				case MYSQL_TYPE_TINY_BLOB:
				case MYSQL_TYPE_MEDIUM_BLOB:
				case MYSQL_TYPE_BLOB:
				case MYSQL_TYPE_LONG_BLOB:
					value->values                    = BlobSource(16);
					resultBindings[i].buffer_type    = MYSQL_TYPE_LONG_BLOB;
					value->cellFactory = [](BindHolder *bind){
						BlobSource &container = std::get<BlobSource>(bind->values);
						auto blobBuffer = std::make_shared<boost::iostreams::stream_buffer<BlobSource>>(container);
						return ecs::tools::any::make_unique<ecs::db3::types::Blob>(blobBuffer);
					};
					break;
				default:
					setErrorString("Unsupported field type");
					return -1;
			}

			resultValues.push_back(std::move(value));
		}

		/* Now attach the result binding the the statement */
		rc = mysql_stmt_bind_result(statement.get(), resultBindings.data());
		if(rc != 0) {
			return -1;
		}

		return 0;
	}else{
		return -1;
	}
}

void ecs::db3::MariaDBStatement::bindBlob(
		const std::shared_ptr<std::basic_streambuf<char> > &blob,
		std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>> &value) {
	auto blobData = std::make_shared<std::vector<char>>();

	/* This is the stream where the data is in. We use the buffer which the user gave us. */
	std::istream        blobStream(blob.get());

	blobData->assign(std::istreambuf_iterator<char>(blobStream), std::istreambuf_iterator<char>());

	value.first.buffer_type   = MYSQL_TYPE_BLOB;
	value.first.buffer        = blobData->data();
	value.first.buffer_length = blobData->size();
	value.first.is_null       = 0;
	value.second              = std::make_unique<ecs::db3::types::cell_T>(blobData);
}

void ecs::db3::MariaDBStatement::bindBlob(
		const std::shared_ptr<std::basic_istream<char>> &blob,
		std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>> &value) {
	auto blobData = std::make_shared<std::vector<char>>();

	blobData->assign(std::istreambuf_iterator<char>(*blob), std::istreambuf_iterator<char>());

	value.first.buffer_type   = MYSQL_TYPE_BLOB;
	value.first.buffer        = blobData->data();
	value.first.buffer_length = blobData->size();
	value.first.is_null       = 0;
	value.second              = std::make_unique<ecs::db3::types::cell_T>(blobData);
}

bool ecs::db3::MariaDBStatement::bind(ecs::db3::types::cell_T *parameter,
		const std::string *parameterName, int n) {
	std::scoped_lock lock(connection->connectionMutex);

	/* When type conversion happens the reference must be valid so
	 * storing a unique pointer to the new value
	 * will keep that value alive.
	 */
	std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>> value;
	std::memset(&value.first, '\0', sizeof(MYSQL_BIND));

	switch(parameter->getTypeId()) {
		case types::typeId::int64_T:
			value.first.buffer_type   = MYSQL_TYPE_LONGLONG;
			value.first.buffer        = reinterpret_cast<char*>(parameter->cast<std::int64_t>());
			value.first.buffer_length = sizeof(std::int64_t);
			value.first.is_unsigned   = false;
			break;
		case types::typeId::uint64_T:
			value.second = std::make_unique<ecs::db3::types::cell_T>(std::int64_t(parameter->cast<std::uint64_t>()));
			value.first.flags        |= UNSIGNED_FLAG;
			value.first.is_unsigned   = true;
			value.first.buffer_type   = MYSQL_TYPE_LONGLONG;
			value.first.buffer        = reinterpret_cast<char*>(value.second->cast<std::uint64_t>());
			value.first.buffer_length = sizeof(std::uint64_t);
			break;
		case types::typeId::string:
			value.first.buffer_type   = MYSQL_TYPE_STRING;
			value.first.buffer        = reinterpret_cast<char*>(parameter->cast<std::string>()->data());
			value.first.buffer_length = parameter->cast<std::string>()->size();
			break;
		case types::typeId::double_T:
			value.first.buffer_type   = MYSQL_TYPE_DOUBLE;
			value.first.buffer        = reinterpret_cast<char*>(parameter->cast<double>());
			value.first.buffer_length = sizeof(double);
			value.first.is_unsigned   = false;
			break;
		case types::typeId::float_T:
			value.first.buffer_type   = MYSQL_TYPE_FLOAT;
			value.first.buffer        = reinterpret_cast<char*>(parameter->cast<float>());
			value.first.buffer_length = sizeof(float);
			value.first.is_unsigned   = false;
			break;
		case types::typeId::null:
			value.first.buffer_type   = MYSQL_TYPE_NULL;
			break;
		case types::typeId::blob:
			bindBlob(parameter->cast_reference<ecs::db3::types::Blob::type>(), value);
			break;
		case types::typeId::blobInput:
			bindBlob(parameter->cast_reference<ecs::db3::types::BlobInput::type>(), value);
			break;
		case types::typeId::boolean_T:
			value.second = std::make_unique<ecs::db3::types::cell_T>(std::int8_t(parameter->cast_reference<bool>() == true ? 1 : 0));
			value.first.buffer_type   = MYSQL_TYPE_TINY;
			value.first.buffer        = reinterpret_cast<char*>(parameter->cast<std::int8_t>());
			value.first.buffer_length = sizeof(std::int8_t);
			value.first.is_unsigned   = false;
			break;
		default:
			setErrorString("Unsupported data type");
			return false;
	}

	parameterBindings.params.push_back(value.first);
	parameterBindings.cells.push_back(std::move(value.second));

	return true;
}

void ecs::db3::MariaDBStatement::reset() {
	std::scoped_lock lock(connection->connectionMutex);
	auto rc = mysql_stmt_reset(statement.get());
	parameterBindings.reset();
}

void ecs::db3::MariaDBStatement::clearBindings() {
	std::scoped_lock lock(connection->connectionMutex);
}

ecs::db3::Row::uniquePtr_T ecs::db3::MariaDBStatement::fetch() {
	std::scoped_lock lock(connection->connectionMutex);

	auto result = std::make_unique<ecs::db3::Row>();

	if(!metaResult) return ecs::db3::Row::uniquePtr_T();

	/* Get column count */
	auto resultColumnCount = mysql_num_fields(metaResult.get());

	/* Fetch a single row. Then we need to check if there are
	 * columns which did not fetch completely.
	 */
	auto rc = mysql_stmt_fetch(this->statement.get());

	if(rc == 0 || rc == MYSQL_DATA_TRUNCATED) {
		/* When data is truncated we must fetch each
		 * individual column where the data did not fit.
		 */
		for(unsigned int i = 0;i < resultColumnCount;++i) {
			if(*resultBindings[i].is_null) {
				*result << ecs::tools::any::make_unique<ecs::db3::types::Null>();
				continue;
			}else if(resultBindings[i].buffer == nullptr) {
				/* Let's go thru every column binding and resize
				 * that column when it needs to be resized
				 */
				resultValues.at(i)->resize(*resultBindings[i].length);
				rc = mysql_stmt_fetch_column(statement.get(), &resultBindings[i], i, 0);
				*result << resultValues.at(i)->getCell();

				/* Reset is required here because the buffer is allocated after
				 * knowing the actual size.
				 * The next call to fetch column has another buffer size which we don't
				 * know yet.
				 */
				resultValues.at(i)->reset();
			}else{
				rc = mysql_stmt_fetch_column(statement.get(), &resultBindings[i], i, 0);
				*result << resultValues.at(i)->getCell();
			}

			if(rc) {
				setErrorString(mysql_stmt_error(statement.get()));
				return ecs::db3::Row::uniquePtr_T();
			}
		}

	}else if(rc == MYSQL_NO_DATA) {
		setErrorString(mysql_stmt_error(statement.get()));
		/* In this case there is no data and the row is empty */
		return ecs::db3::Row::uniquePtr_T();
	}

	return result;
}

ecs::db3::MariaDBConnection::MariaDBConnection() {
	std::scoped_lock lock(libraryInitMutex);
	if(libraryInit == false) {
		mysql_library_init(0, nullptr, nullptr);
		libraryInit = true;
	}
}

ecs::db3::MariaDBConnection::~MariaDBConnection() {

}

std::string ecs::db3::MariaDBConnection::getPluginVersion() {
	return "0.0.0";
}

std::string ecs::db3::MariaDBConnection::getPluginDescription() {
	return "MariaDB Connection";
}

std::string ecs::db3::MariaDBConnection::getPluginAuthor() {
	return "Geoffrey Mellar";
}

std::string ecs::db3::MariaDBConnection::getPluginName() {
	return "mariadb";
}

ecs::db3::StatementImpl::ptr_T ecs::db3::MariaDBConnection::prepare(
		const std::string &query) {
	if(!connection) {
		return nullptr;
	}

	auto result = std::make_unique<MariaDBStatement>(connection, query);

	if(result->statement) {
		return result.release();
	}else{
		setErrorMessage(result->getErrorString());
		return nullptr;
	}
}

ecs::db3::MigratorImpl* ecs::db3::MariaDBConnection::getMigrator(
		DbConnection *connection) {
	auto result = std::make_unique<MigratorImplMariaDb>(connection);
	return result.release();
}

bool ecs::db3::MariaDBConnection::connect(
		const ConnectionParameters &parameters) {
	connection = std::make_shared<ConnectionWrapper>(parameters);
	auto result = connection->connection == nullptr;

	if(result) {
		setErrorMessage(connection->errorMessage);
	}

	return result == false;
}

bool ecs::db3::MariaDBConnection::disconnect() {
	connection.reset();
	return true;
}

void ecs::db3::MariaDBConnection::startTransation() {
	int rc = mysql_autocommit(connection->connection.get(), false);

	if (rc != 0) {
		// retry if connection lost
		int err = mysql_errno(connection->connection.get());
		if (err == 2006 || err == 2013) {
			rc = mysql_autocommit(connection->connection.get(), false);
		}
	}
}

bool ecs::db3::MariaDBConnection::execute(const std::string &query) {
	int rc = mysql_real_query(connection->connection.get(), query.data(), query.size());
	return rc == 0;
}

void ecs::db3::MariaDBConnection::commitTransaction() {
	mysql_commit(connection->connection.get());
}

void ecs::db3::MariaDBConnection::rollbackTransaction() {
	mysql_rollback(connection->connection.get());
}


void ecs::db3::MariaDBConnection::autocommit(bool value) {
	int rc = mysql_autocommit(connection->connection.get(), true);

	if (rc != 0) {
		// retry if connection lost
		int err = mysql_errno(connection->connection.get());
		if (err == 2006 || err == 2013) {
			rc = mysql_autocommit(connection->connection.get(), true);
		}
	}
}

DYNLIB_BEGIN_CLASS_DEFINITION()
	DYNLIB_CLASS_DEFINITION("DatabaseConnection", ecs::db3::MariaDBConnection);
DYNLIB_END_CLASS_DEFINITION()
