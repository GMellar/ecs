/*
 * postgresql.cpp
 *
 *  Created on: 13.12.2014
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2017 Geoffrey Mellar <mellar@gamma-kappa.com>
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
 
#include <ecs/database/postgresql/postgresql.hpp>
#include <ecs/database/impl/MigratorImpl.hpp>

/** @addtogroup ecsdb 
 * @{
 */

using namespace ecs::db3;

namespace ecs {
namespace db3 {

class MigratorImplPostgresql : public MigratorImpl {
public:
	MigratorImplPostgresql(DbConnection *connection) : MigratorImpl(connection) {

	}

	virtual ~MigratorImplPostgresql() {

	}

	virtual int initSchema() final {
		/* Table creation works always because we use conditional creation */
		connection->execute(
			"CREATE TABLE IF NOT EXISTS schema_info ("
				"name VARCHAR PRIMARY KEY,"
				"value VARCHAR"
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
		Statement::sharedPtr_T stmt;

		stmt = connection->prepare("SELECT value FROM schema_info WHERE name='version';");
		auto result = stmt->execute();
		auto row    = result.fetch();

		if(!row) throw std::runtime_error("Schema version not available");
		return std::stoi(row->at(0).cast_reference<std::string>(), nullptr, 10);
	}

	virtual bool setSchemaVersion(int version) {
		Statement::sharedPtr_T stmt;

		stmt = connection->prepare("UPDATE schema_info SET value=? WHERE name='version';");
		stmt->bind(std::to_string(version));
		return stmt->execute();
	}

	bool doMigration ( Migrator::Migration::ptr_T migration ) {
		connection->execute("BEGIN EXCLUSIVE TRANSACTION;");

		try {
			if(!migration->upMigration(connection)) {
				connection->execute("ROLLBACK TRANSACTION;");
				return false;
			}
		}catch(std::exception &) {
			connection->execute("ROLLBACK TRANSACTION;");
			throw;
		}

		connection->execute("END TRANSACTION;");
		return true;
	}
};

}
}

void ecs::db3::PostgresqlStatement::PGresultDeleter(PGresult *obj) {
	PQclear(obj);
}

PostgresqlStatement::PostgresqlStatement(PGconn *connection, const std::string &query)
	: result(nullptr, &PGresultDeleter), connection(connection), query(query), iRow(0) {
	if(connection == nullptr) {
		throw std::runtime_error("Connection is invalid");
	}
}
	
PostgresqlStatement::~PostgresqlStatement() {

}
	
Row::uniquePtr_T PostgresqlStatement::fetch() {
	Row::uniquePtr_T row;

	for(std::int64_t iCol = 0;iCol < PQnfields(result.get());++iCol) {
		row.reset(new Row);

		/* Check if the content is null because there is no oid type for
		 * that case.
		 */
		if(PQgetisnull(result.get(), iRow, iCol)) {
			*row << new ecs::db3::types::cell_T(nullptr, types::Null());
			/* There is nothing to do when the value is null */
			continue;
		}

		switch(PQftype(result.get(), iCol)) {
			// We don't care about the size here because we keep everything in 64bit integer
			case INT2OID:
			case INT4OID:
			case INT8OID:
				*row << ecs::tools::any::make<types::Int64>(std::strtoll(PQgetvalue(result.get(), iRow, iCol), nullptr, 10));
				break;
			// We use double for everything
			case FLOAT4OID:
			case FLOAT8OID:
				*row << ecs::tools::any::make<types::Double>(std::strtod(PQgetvalue(result.get(), iRow, iCol), nullptr));
				break;
			case VARCHAROID:
				*row << ecs::tools::any::make<types::String>(PQgetvalue(result.get(), iRow, iCol), PQgetlength(result.get(), iRow, iCol));
				break;
			case BYTEAOID:
			default:
				row.reset();
				break;
		}
	}

	iRow++;
	return std::move(row);
}

int PostgresqlStatement::execute(Table *resultTable) {
	bool rc = true;
	iRow = 0;

	/* Holds the real parameter values */
	std::vector<std::string> stringValues;
	/* Holds all pointers to the parameters values
	 * converted to string
	 */
	std::vector<const char*> paramValues;
	/* Holds the parameter value length */
	std::vector<int>         paramLengths;

	/* Bind all parameters */
	std::for_each(bindings.begin(), bindings.end(), [&](ecs::db3::types::cell_T *cell){
		switch(cell->getTypeId()) {
		case types::typeId::int64_T:
			stringValues.push_back(std::to_string(cell->cast_reference<std::int64_t>()));
			paramValues.push_back(stringValues.back().c_str());
			paramLengths.push_back(0);
			break;
		case types::typeId::uint64_T:
			stringValues.push_back(std::to_string(cell->cast_reference<std::uint64_t>()));
			paramValues.push_back(stringValues.back().c_str());
			paramLengths.push_back(0);
			break;
		case types::typeId::float_T:
			stringValues.push_back(std::to_string(cell->cast_reference<float>()));
			paramValues.push_back(stringValues.back().c_str());
			paramLengths.push_back(0);
			break;
		case types::typeId::double_T:
			stringValues.push_back(std::to_string(cell->cast_reference<double>()));
			paramValues.push_back(stringValues.back().c_str());
			paramLengths.push_back(0);
			break;
		case types::typeId::string:
			stringValues.push_back(""); // We need that empty string to keep the size valid
			paramValues.push_back(cell->cast_reference<std::string>().c_str());
			paramLengths.push_back(0);
			break;
		case types::typeId::null:
			stringValues.push_back(""); // We need that empty string to keep the size valid
			paramValues.push_back(nullptr);
			paramLengths.push_back(0);
			break;
		case types::typeId::blob:

		default:
			rc = false;
			break;
		}
	});

	if(rc == false) {
		setErrorString("Unknow parameter type");
		return -1;
	}

	/* Execute query */
	result.reset(PQexecParams(connection,
						query.c_str(),
						bindings.size(),
						nullptr,
						paramValues.size() ? &paramValues[0] : nullptr,
						paramLengths.size() ? &paramLengths[0] : nullptr,
						nullptr,
						0));

	/* Test the result for errors */
	switch(PQresultStatus(result.get())) {
	case PGRES_COMMAND_OK:
		break;
	default:
		setErrorString(PQresultErrorMessage(result.get()));
		return -1;
	}

	/* Put all column names in the result table */
	for(std::int64_t i = 0;i < PQnfields(result.get());++i) {
		resultTable->columnNames.push_back(PQfname(result.get(), i));
	}

	/* Return successfull result */
	return 0;
}

bool PostgresqlStatement::bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n) {
	bindings.push_back(parameter);
	return true;
}

/** This one has no effect because
 * we prepare the statement everythime.
 */
void PostgresqlStatement::reset() {

}

void PostgresqlStatement::clearBindings() {
	bindings.clear();
}


PostresqlConnection::PostresqlConnection() : connection(nullptr) {
		
}
	
PostresqlConnection::~PostresqlConnection() {
	disconnect();
}
	
std::string PostresqlConnection::getPluginVersion() {
	return "0.0.0";
}

std::string PostresqlConnection::getPluginDescription() {
	return "Postgresql database plugin";
}

std::string PostresqlConnection::getPluginAuthor() {
	return "Geoffrey Mellar <mellar@gamma-kappa.com>";
}

std::string PostresqlConnection::getPluginName() {
	return "database.postgresql";
}

StatementImpl::ptr_T PostresqlConnection::prepare(const std::string &query) {
	try {
		return StatementImpl::uniquePtr_T(new PostgresqlStatement(connection, query)).release();
	}catch(...) {
		return nullptr;
	}

}

ecs::db3::MigratorImpl* ecs::db3::PostresqlConnection::getMigrator(
		DbConnection *connection) {
	return new ecs::db3::MigratorImplPostgresql(connection);
}

bool PostresqlConnection::connect(const ConnectionParameters &parameters) {
	/* Serialize connection parameters */
	std::vector<const char *> key;
	std::vector<const char *> value;
	
	/* Open the connetion to the database */
	connection = PQsetdbLogin(parameters.getHostname().c_str(),
		std::to_string(parameters.getPort()).c_str(),
		nullptr,
		nullptr,
		parameters.getDbName().c_str(),
		parameters.getDbUser().c_str(),
		parameters.getPassword().c_str());
		
	if(connection == nullptr) {
		setErrorMessage("No connection to close");
		return false;
	}

	/* Check if the connection is good */
	if(PQstatus(connection) == CONNECTION_BAD) {
		setErrorMessage(PQerrorMessage(connection));
		PQfinish(connection);
		connection = nullptr;
		return false;
	}
	
	return true;
}

bool PostresqlConnection::disconnect() {
	if(connection == nullptr){
		return false;
	}
	/* Pqfinish does not return any value
	 * so we must assume that it succeeds.
	 */
	PQfinish(connection);
	connection = nullptr;
	return true;
}

DYNLIB_BEGIN_CLASS_DEFINITION()
	DYNLIB_CLASS_DEFINITION("DatabaseConnection", PostresqlConnection);
	DYNLIB_END_CLASS_DEFINITION()
/** @} */
