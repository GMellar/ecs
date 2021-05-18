/*
 * Connector.cpp
 *
 *  Created on: 06.12.2014
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

#include <ecs/database/Connector.hpp>
#include <ecs/config.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/DbConnectionImpl.hpp>
#include <ecs/Library.hpp>
#include <exception>
#include <ecs/database/Exception.hpp>

#include <ecs/database/sqlite3/sqlite3.hpp>
#include <ecs/database/postgresql/postgresql.hpp>

using namespace ecs::db3;

ecs::db3::PluginLoader::PluginLoader() {

}

ecs::db3::PluginLoader::~PluginLoader() {

}

DbConnection::ptr_T ecs::db3::PluginLoader::loadPtr(
		const ConnectionParameters &params) {
	/* Library object which is loadable */
	ecs::dynlib::Library::sharedPtr_T library;
	
	/* Create a shared pointer of a database connection interface.
	 * This will contain the implementation which is the
	 * loadable plugin.
	 */
	std::unique_ptr<DbConnection> result(new ecs::db3::DbConnection());
	
	if(params.getBackend() == "sqlite3") {
#ifdef ECS_SQLITE3_DRIVER
		auto module   = ecs::dynlib::Class<ConnectionImpl>(new ecs::db3::Sqlite3Connection());
		result->impl->module = module;
		result->impl->parameters = params;
#else
		throw exceptions::Exception("sqlite3 driver not enabled");
#endif
	}else if(params.getBackend() == "postgresql") {
#ifdef ECS_POSTGRESQL_DRIVER
		auto module   = ecs::dynlib::Class<ConnectionImpl>(new ecs::db3::PostresqlConnection());
		result->impl->module = module;
		result->impl->parameters = params;
#else
		throw exceptions::Exception("postgresql driver not enabled");
#endif
	}else{
		/* The default plugin loading scheme is to look inside 
		* the default plugin directory and open the backend file with 
		* the appropriate extension. 
		*/
		library = ecs::dynlib::Library::load(params.getPluginDirectory() + "/" + params.getBackend() + params.getPluginExtension());

		/* Create the implementation of the database connection */
		auto module   = ecs::dynlib::Library::loadClass<ConnectionImpl>(library, "DatabaseConnection");

		if(module && result){
			/* Store implementation because every call needs the implementation */
			result->impl->module = module;
			/* Copy connection parameters */
			result->impl->parameters = params;
		}else{
			throw exceptions::Exception("DbConnection construction failed");
		}
	}
	
	/* Connect the database */
	if(!result->impl->module->connect(params)) {
		throw exceptions::Exception("Connecting to the database not possible: " + result->impl->module->getErrorMessage());
	}

	/* Return connection */
	return result.release();
}
