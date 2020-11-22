/*
 * ConnectionParameters.hpp
 *
 *  Created on: 14.12.2014
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

#ifndef DATABASECONNECTIONPARAMETERS_HPP_
#define DATABASECONNECTIONPARAMETERS_HPP_

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <string>
#include <memory>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb
 * @{
 */

class DbConnection;
class ConnectionParametersImpl;

class ECS_EXPORT ConnectionParameters  {
public:
	POINTER_DEFINITIONS(ConnectionParameters);

	ConnectionParameters();
	ConnectionParameters(const ConnectionParameters &other);
	ConnectionParameters(ConnectionParameters &&other);
	ConnectionParameters &operator=(const ConnectionParameters &other);
	ConnectionParameters &operator=(ConnectionParameters &&other);
	virtual ~ConnectionParameters();

	void erasePassword();

	const std::string& getDbName() const;

	void setDbName(const std::string& dbname);

	const std::string& getDbFilename() const;

	void setDbFilename(const std::string& filename);

	const std::string& getHostname() const;

	void setHostname(const std::string& hostname);

	void setDbPassword(const std::string& password);

	int getPort() const;

	void setPort(int port);

	const std::string& getDbUser() const;

	void setDbUser(const std::string& user);

	void setBackend(const std::string& dbBackend);
	
	const std::string& getBackend() const;

	const std::string& getFilename() const;

	const std::string& getPassword() const;

	const std::string& getUser() const;

	const std::string& getPluginDirectory() const;
	
	void setPluginDirectory(const std::string &dir);
	
	const std::string& getPluginExtension() const;
	
	void setPluginExtension(const std::string &e);
	
	inline std::shared_ptr<DbConnection> connect() {
		return std::shared_ptr<DbConnection>(connectPtr());
	}

protected:
	/** Convenience function to establish a connection with
	 * the parameters from this class.
	 * Internally this uses the plugin loader which you
	 * would use manually.
	 */
	DbConnection *connectPtr();

private:
	ConnectionParametersImpl *impl;
};

/** @} */

}
}

#endif /* DATABASECONNECTIONPARAMETERS_HPP_ */
