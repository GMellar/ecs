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

#ifndef ECSDB_MIGRATOR_MIGRATORIMPL_H
#define ECSDB_MIGRATOR_MIGRATORIMPL_H

#include <ecs/database/Migrator.hpp>

namespace ecs {
namespace db3 {
	
/** This is the implementation class for the migrator backend. This 
 * will only be created inside a migration and the Migrator pointer will always be valid. 
 * Never pass the migrator pointer to anything because it will be invalid as soon as 
 * the migration is done. 
 *
 */
class MigratorImpl {
public:
	POINTER_DEFINITIONS(MigratorImpl);
	
	MigratorImpl(DbConnection::ptr_T connection);

	virtual ~MigratorImpl();
	
	/** Implement this to initialize the schema. 
	 *
	 */
	virtual int  initSchema() = 0;

	/** Implement this function for schema version 
	 * reading. This is protected because it is not intended
	 * to use it outside the migrator. Every schema version read 
	 * should happen in a safe environment where the version does 
	 * not alter after readind. 
	 */
	virtual int  getSchemaVersion() = 0;

	/** Set the schema version. This 
	 * function shouldn't be called directly but within 
	 * a transaction. We use protected here 
	 * because the user should not have access to 
	 * schema modification functions. Use this after the schema 
	 * upgrade was successful.
	 */
	virtual bool setSchemaVersion(int version) = 0;

	virtual bool doMigration ( Migrator::Migration::ptr_T migration ) = 0;
protected:
	DbConnection::ptr_T connection;
};

}
}

#endif // ECSDB_MIGRATOR_MIGRATORIMPL_H
