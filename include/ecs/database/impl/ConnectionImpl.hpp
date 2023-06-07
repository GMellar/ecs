/*
 * ConnectionImpl.hpp
 *
 *  Created on: 24.02.2016
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

#ifndef SRC_ECSDB_DB3_IMPL_CONNECTIONIMPL_HPP_
#define SRC_ECSDB_DB3_IMPL_CONNECTIONIMPL_HPP_


#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/Statement.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/ConnectionParameters.hpp>
#include <ecs/Library.hpp>
#include <memory>
#include <mutex>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb 
 * @{
 */

class MigratorImpl;

class ECS_EXPORT ConnectionImpl : public ecs::dynlib::LoadableClass {
public:
	POINTER_DEFINITIONS(ConnectionImpl);

	ConnectionImpl();

	ConnectionImpl(const ConnectionImpl &connection) = default;

	ConnectionImpl(ConnectionImpl &&connection) = default;

	ConnectionImpl &operator=(const ConnectionImpl &connection) = default;

	ConnectionImpl &operator=(ConnectionImpl &&connection) = default;

	virtual ~ConnectionImpl();

	virtual bool connect(const ConnectionParameters &parameters);

	virtual bool disconnect();
	
	/** Execute a single statement which does not 
	 * need any return values or parameter bindings. You should implement this 
	 * because the default behaviour is to use a statement 
	 * which must be implemented. 
	 */
	virtual bool execute(const std::string &query);
	
	/** Create a new statement from a query.
	 * 
	 */
	virtual StatementImpl::ptr_T prepare(const std::string &query) = 0;
	
	/** Get the implementation for the migrator. Plugin developers 
	 * may provide their own migrator. Inside the plugin.
	 */
	virtual ecs::db3::MigratorImpl* getMigrator(DbConnection *connection) = 0;
	
	virtual std::string getErrorMessage();
	
	void setErrorMessage(const std::string &message);

protected:
	std::mutex           errorMessageMutex;
	std::string          errorMessage;
	ConnectionParameters connectionParameters;
};

/** @} */

}
}

#endif /* SRC_ECSDB_DB3_IMPL_CONNECTIONIMPL_HPP_ */
