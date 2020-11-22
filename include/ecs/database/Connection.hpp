/*
 * Connection.cpp
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

#ifndef _DBCONNECTION_HPP_
#define _DBCONNECTION_HPP_

#include <ecs/config.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/Library.hpp>
#include <ecs/database/ConnectionParameters.hpp>
#include <ecs/database/Statement.hpp>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb
 * @{
 */

class ConnectionImpl;
class MigratorImpl;
class DbConnectionImpl;
class Statement;

/** This class represents a database connection. The backend is fully 
 * hidden to the user and resides as shared pointer inside the class. 
 * The implementation is a loadable module. These modules contain a class 
 * which handles the connection. 
 * 
 * It is not possible to use this class directly because this object does 
 * not know how to load the implementation. Instead you must use the 
 * PluginLoader class which builds this class depending on the connection 
 * parameters. 
 *
 * Cloning a connection is very easy and can be done with the integrated connection
 * parameters. Just use get parameters and call the connect function of the
 * connection parameter class.
 */
class ECS_EXPORT DbConnection {
	friend class PluginLoader;
	friend class MigratorImpl;
	friend class Statement;
	friend class StatementInternals;
public:
	POINTER_DEFINITIONS(DbConnection);

	virtual ~DbConnection();

	DbConnection(const DbConnection &connection) = delete;

	DbConnection(DbConnection &&connection) = delete;

	DbConnection &operator=(const DbConnection &connection) = delete;

	DbConnection &operator=(DbConnection &&connection) = delete;
	
	std::shared_ptr<MigratorImpl> getMigrator();
	
	/** Executes a single query string without any 
	 * specific parameters. This function may throw 
	 * when internal prepared statement creation failed. 
	 * 
	 * Depending on the backend, this function may throw when 
	 * the query string is malformed because some backends do 
	 * not allow that. When some other error occured and the 
	 * function did not throw, false is returned. 
	 */
	bool execute(const std::string &query);
	
	/** Get the connection parameters used to build this connection. 
	 * You can start a new connection from the parameters by calling 
	 * the connect function from the parameters class. 
	 */
	const ecs::db3::ConnectionParameters &getParameters() const;
	
	inline Statement::sharedPtr_T prepare(const std::string &query) {
		return Statement::sharedPtr_T(preparePtr(query));
	}

	inline Statement::sharedPtr_T prepareFromFile(const std::string &filename) {
		return Statement::sharedPtr_T(prepareFromFilePtr(filename));
	}

protected:
	/** Implementation details for the connection.
	 *
	 */
	DbConnectionImpl *impl;
	
	/** Make a statement from a query. The returned statement can
	 * exist without keeping the database connection somewhere beause
	 * this class is contained inside the statement.
	 *
	 * Internally this calls the implementation provided function
	 * to build a statement and stores the statement implementation
	 * inside the returned class.
	 */
	Statement::ptr_T preparePtr(const std::string &query);

	/** Opens the specified file and creates a statement.
	 * @see prepare()
	 */
	Statement::ptr_T prepareFromFilePtr(const std::string &filename);
private:
	/** It is not possible for a library user to use 
	 * this class directly because the implementation 
	 * has to be loaded from a module. 
	 */
	DbConnection();
};

/** @} */

}
}

#endif /* DBCONNECTION_HPP_ */
