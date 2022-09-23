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
#include <map>
#include <string>
#include <ecs/database/Exception.hpp>

#ifdef ECS_SQLITE3_DRIVER
#include <ecs/database/sqlite3/sqlite3.hpp>
#endif

#ifdef ECS_POSTGRESQL_DRIVER
#include <ecs/database/postgresql/postgresql.hpp>
#endif

using namespace ecs::db3;

static std::map<std::string, std::function<ecs::dynlib::Class<ConnectionImpl>()>> staticModuleLoader = {
		/* This is here to prevent compiler errors when none of the backends are
		 * enabled.
		 */
		{"", [](){return ecs::dynlib::Class<ConnectionImpl>();}}
#ifdef ECS_SQLITE3_DRIVER
		,{"sqlite3", [](){return ecs::dynlib::Class<ConnectionImpl>(new ecs::db3::Sqlite3Connection());}}
#endif
#ifdef ECS_POSTGRESQL_DRIVER
		,{"postgresql", [](){return ecs::dynlib::Class<ConnectionImpl>(new ecs::db3::PostresqlConnection());}}
#endif
};

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
	ecs::dynlib::Class<ConnectionImpl> module;

	/* Look for builtin modules first */
	module = staticModuleLoader.at(params.getBackend())();

	if(module) {
		result->impl->module = module;
		result->impl->parameters = params;
		return connect(std::move(result), params);
	}

	/* The default plugin loading scheme is to look inside
	* the default plugin directory and open the backend file with
	* the appropriate extension.
	*/
	library = ecs::dynlib::Library::load(params.getPluginDirectory() + "/" + params.getBackend() + params.getPluginExtension());

	/* Create the implementation of the database connection */
	module  = ecs::dynlib::Library::loadClass<ConnectionImpl>(library, "DatabaseConnection");

	if(module && result){
		/* Store implementation because every call needs the implementation */
		result->impl->module     = module;
		/* Copy connection parameters */
		result->impl->parameters = params;
	}else{
		throw exceptions::Exception("DbConnection construction failed");
	}
	
	return connect(std::move(result), params);
}

DbConnection::ptr_T ecs::db3::PluginLoader::connect(
		std::unique_ptr<DbConnection> con,
		const ConnectionParameters &params) {
	/* Connect the database */
	if(!con->impl->module->connect(params)) {
		throw exceptions::Exception("Connecting to the database not possible: " + con->impl->module->getErrorMessage());
	}

	/* Return connection */
	return con.release();
}
