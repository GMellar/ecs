/*
 * Copyright (C) 2017  Geoffrey Mellar <mellar@gamma-kappa.com>
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

#include <ecs/database/impl/MigratorImpl.hpp>
#include <ecs/database/Exception.hpp>

namespace ecs {
namespace db3 {
	
class MigratorImplSqlite3 : public MigratorImpl {
public:
	MigratorImplSqlite3(DbConnection *connection) : MigratorImpl(connection) {
		
	}
	
	virtual ~MigratorImplSqlite3() {
		
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
