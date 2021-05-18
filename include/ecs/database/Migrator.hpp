/*
 * Migrator.hpp
 *
 * Author: Geoffrey Mellar <mellar@gamma-kappa.com>
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

#ifndef MIGRATOR_H
#define MIGRATOR_H

#include <ecs/database/Connection.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/Function.hpp>
#include <ecs/config.hpp>
#include <map>
#include <exception>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb 
 * @{
 */

/** Base class for all Migrator classes.
 * Use this to implement your own migrator. 
 * A Migrator should be used at the application startup 
 * and then be destroyed when all work is done.
 */
class ECS_EXPORT Migrator {
public:
	POINTER_DEFINITIONS(Migrator);
	using migrationFn_t = ecs::tools::Function<bool(Migrator&, DbConnection*)>;
	
	/** An implementation for a single migration 
	 * from a source version to a destination version.
	 * The main idea behind this class is to hold 
	 * a function pointer for the up migration. 
	 */
	class ECS_EXPORT Migration {
		friend class Migrator;
	public:
		POINTER_DEFINITIONS(Migration);
			
		virtual ~Migration();
		
		/** Needs to return true when the migration was successful.
		* You can also throw inside this function and nothing is changed 
		* in the database. 
		*/
		virtual bool upMigration(ecs::db3::Migrator& migrator, ecs::db3::DbConnection* connection) = 0;
		
		/** Returns the source version */
		int getFromVersion() const;
		
		/** Returns the destination version of the migration */
		int getToVersion() const;
	protected:
		/** Source version of the database */
		int from_version;
		
		/** Destination version after this migration 
		 * altered the database successfully.
		 */
		int to_version;
		
		/** Constructing this migration is only possible 
		* when this class is used as base class and 
		* you implement the upMigration function.
		*/
		Migration(int from_version, int to_version);
	};
	
	/** Using this class implements a simple interface to migrations 
	* where a function pointer is used. It is still more elegant to use 
	* the Migration class as a base class for your migration 
	* implementation. 
	*/
	class ECS_EXPORT MigrationFunction : public Migration {
	public:
		MigrationFunction(int from, int to, std::function<bool(Migrator&, DbConnection*)> fn);
		virtual ~MigrationFunction();
		virtual bool upMigration(Migrator& migrator, DbConnection* connection) final;
	protected:
		std::function<bool(Migrator&, DbConnection*)> fn;
	};
	
	/** Use shared pointer of the connection here 
	 * because every migration needs a connection. 
	 * A migration lifetime should be as short as possible. 
	 * 
	 * This means as long as the Migrator is active the connection 
	 * is kept open. 
	 */
	Migrator();
	
	virtual ~Migrator();
	
	void addMigration(Migration::ptr_T migration);
	
	void addMigration(Migration::uniquePtr_T migration);
	
	/** This is the only function which is accessible to 
	 * the user because schema version writing and reading 
	 * must be within a transaction. Ther might be cases where 
	 * a migration is called at the same time so reading 
	 * the schema must return the real value.
	 * 
	 * @return 0 if the migration was successful. If the result is -1 then there is no 
	 * implementation for the database which means you have to subclass the migrator class 
	 * to implement your own migration. When using this function from a subclass you should call this method 
	 * first and start your migration when return value is -1. This results in a safe way of handling future 
	 * implementation of the migration without interfering with your code. 
	 */
	virtual int startMigration(DbConnection::sharedPtr_T connection);
	
	/** Initialized the schema for the database connection. This function 
	 * can be called as often as possible without damaging the database. It 
	 * will create the schema when not present or behave as no operatio 
	 * when the schema structure is present. 
	 */
	virtual int initSchema(DbConnection::sharedPtr_T connection);
protected:
	/** Maps which contains all migration objects where the key 
	 * is the current database version which needs altering. 
	 */
	std::map<int, Migration::uniquePtr_T> migrations;
	
	/** Get a destination for the current source version which updates 
	 * the database to a destination version. Call this function as long as there 
	 * is no more result. 
	 */
	Migration::ptr_T getMigration(int fromVersion);
	
private:
	
};

/** @} */

}
}
#endif // MIGRATOR_H
