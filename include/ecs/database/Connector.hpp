/*
 * Connector.hpp
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

#ifndef DATABASE_CONNECTOR_HPP_
#define DATABASE_CONNECTOR_HPP_

#include <ecs/config.hpp>
#include <memory>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/ConnectionParameters.hpp>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb
 * @{
 */

class MigratorImpl;

/** Plugin loader class for a database plugin defined
 * by the database parameters. Before you can make any 
 * database connection you need to load a plugin. Plugins 
 * are shared libraries which handle database 
 * connections. 
 */
class ECS_EXPORT PluginLoader {
	friend class ConnectionParameters;
public:
	PluginLoader();

	PluginLoader(const PluginLoader &loader) = default;

	PluginLoader(PluginLoader &&loader) = default;

	PluginLoader &operator=(const PluginLoader& loader) = default;

	PluginLoader &operator=(PluginLoader &&loader) = default;
	
	virtual ~PluginLoader();
	
	/** Inline function to prevent standard library containers be compiled
	 * into the shared library. Whoever used this function will use the shared
	 * pointer instantiation of its own standard library.
	 */
	inline DbConnection::sharedPtr_T load(const ConnectionParameters &params) {
		return DbConnection::sharedPtr_T(this->loadPtr(params));
	}

protected:
	/** Loads the database connection depending on the
	 * connection parameters. It will store the connection
	 * parameters inside the loaded connection so there is
	 * always the possibility to get all the parameters.
	 */
	DbConnection::ptr_T loadPtr(const ConnectionParameters &params);
};

/** @} */

}
}

#endif /* DATABASECONNECTOR_HPP_ */
