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
#include <ecs/database/impl/DbConnectionImpl.hpp>
#include <ecs/database/Migrator.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/Exception.hpp>

MigratorImpl::MigratorImpl ( Migrator::ptr_T migrator, DbConnection::sharedPtr_T connection ) : connection(connection), migrator(migrator) {

}

MigratorImpl::~MigratorImpl() {

}

bool MigratorImpl::doMigration ( Migrator::Migration::ptr_T migration ) {
	return migration->upMigration(*migrator, connection.get());
}

MigratorImpl::sharedPtr_T MigratorImpl::getMigrator (Migrator::ptr_T migrator, DbConnection::sharedPtr_T con ) {
	/* Check if there are builtin migrators */
	if(con->getParameters().getBackend() == "sqlite3"){
		return std::make_shared<MigratorImplSqlite3>(migrator, con);
	}
	
	/* Check if there are plugin provided migrators */
	{
		auto m = con->impl->module->getMigrator();
		if(m) {
			return m;
		}
	}
	
	throw exceptions::Exception("Database schema migration not possible because of an unknown backend");
}
