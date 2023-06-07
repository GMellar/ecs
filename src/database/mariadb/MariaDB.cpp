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

std::mutex ecs::db3::MariaDBConnection::libraryInitMutex;
bool ecs::db3::MariaDBConnection::libraryInit = false;



ecs::db3::MariaDBConnection::ConnectionWrapper::ConnectionWrapper(
		const ConnectionParameters &parameters) {
	std::scoped_lock lock(connectionMutex);

	connection.reset(mysql_init(nullptr), [](MYSQL *connection){
		mysql_close(connection);
	});

	if(connection) {
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

	auto rc = mysql_stmt_prepare(statement.get(), query.c_str(), query.size());
}

ecs::db3::MariaDBStatement::~MariaDBStatement() {
	statement.reset();
	connection.reset();
}

int ecs::db3::MariaDBStatement::execute(Table *table) {
	if(statement) {
		std::scoped_lock lock(connection->connectionMutex);
		auto rc = mysql_stmt_execute(statement.get());

		/* Fetch meta result */
		metaResult.reset(mysql_stmt_result_metadata(statement.get()), [](MYSQL_RES *meta){
			mysql_free_result(meta);
		});

		/* Fetch the column count of the result */
		auto resultColumnCount = mysql_num_fields(metaResult.get());

		return 0;
	}else{
		return -1;
	}
}

bool ecs::db3::MariaDBStatement::bind(ecs::db3::types::cell_T *parameter,
		const std::string *parameterName, int n) {
	std::scoped_lock lock(connection->connectionMutex);
	std::unique_ptr<ecs::db3::types::cell_T> cell;

	MYSQL_BIND bindValue;
	std::memset(&bindValue,0,sizeof(MYSQL_BIND));

	switch(parameter->getTypeId()) {
		case types::typeId::int64_T:
			bindValue.buffer_type  = MYSQL_TYPE_LONGLONG;
			bindValue.buffer       = reinterpret_cast<char*>(parameter->cast<std::int64_t>());
			bindValue.length_value = sizeof(std::int64_t);
			bindValue.is_null      = 0;
			break;
		case types::typeId::uint64_T:
			cell = std::make_unique<ecs::db3::types::cell_T>(std::int64_t(parameter->cast<std::uint64_t>()));
			bindValue.buffer_type  = MYSQL_TYPE_LONGLONG;
			bindValue.buffer       = reinterpret_cast<char*>(cell->cast<std::uint64_t>());
			bindValue.length_value = sizeof(std::int64_t);
			bindValue.is_null      = 0;
			break;
		case types::typeId::string:
			bindValue.buffer_type  = MYSQL_TYPE_STRING;
			bindValue.buffer       = reinterpret_cast<char*>(parameter->cast<std::string>()->data());
			bindValue.length_value = parameter->cast<std::string>()->size();
			bindValue.is_null      = 0;
			break;
		case types::typeId::double_T:
			bindValue.buffer_type  = MYSQL_TYPE_DOUBLE;
			bindValue.buffer       = reinterpret_cast<char*>(parameter->cast<double>());
			bindValue.length_value = sizeof(double);
			bindValue.is_null      = 0;
			break;
		case types::typeId::float_T:
			bindValue.buffer_type  = MYSQL_TYPE_FLOAT;
			bindValue.buffer       = reinterpret_cast<char*>(parameter->cast<float>());
			bindValue.length_value = sizeof(float);
			bindValue.is_null      = 0;
			break;
		case types::typeId::null:
			bindValue.buffer_type = MYSQL_TYPE_NULL;
			break;
		case types::typeId::blob:
			//bindBLOB(sqlite3Stmt.get(), n+1, any::cast_reference<Blob>(*parameter));
			break;
		case types::typeId::blobInput:
			//bindIstream(sqlite3Stmt.get(), n+1, any::cast_reference<BlobInput>(*parameter));
			break;
		case types::typeId::boolean_T:
			cell = std::make_unique<ecs::db3::types::cell_T>(std::int64_t(parameter->cast_reference<bool>() == true ? 1 : 0));
			bindValue.buffer_type  = MYSQL_TYPE_LONGLONG;
			bindValue.buffer       = reinterpret_cast<char*>(parameter->cast<std::int64_t>());
			bindValue.length_value = sizeof(std::int64_t);
			bindValue.is_null      = 0;
			break;
		default:
			return false;
	}

	params.push_back(std::make_pair(bindValue, std::move(cell)));
	auto rc = mysql_stmt_bind_param(statement.get(), &params.back().first);

	return true;
}

void ecs::db3::MariaDBStatement::reset() {
	std::scoped_lock lock(connection->connectionMutex);
	mysql_stmt_reset(statement.get());
	params.clear();
}

void ecs::db3::MariaDBStatement::clearBindings() {
	std::scoped_lock lock(connection->connectionMutex);
}

ecs::db3::Row::uniquePtr_T ecs::db3::MariaDBStatement::fetch() {
	auto result = std::make_unique<ecs::db3::Row>();

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
	return result.release();
}

ecs::db3::MigratorImpl* ecs::db3::MariaDBConnection::getMigrator(
		DbConnection *connection) {
	return nullptr;
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

DYNLIB_BEGIN_CLASS_DEFINITION()
	DYNLIB_CLASS_DEFINITION("DatabaseConnection", ecs::db3::MariaDBConnection);
DYNLIB_END_CLASS_DEFINITION()
