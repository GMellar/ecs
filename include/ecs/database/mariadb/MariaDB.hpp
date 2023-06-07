/*
 * MariaDB.hpp
 *
 *  Created on: 07.06.2023
 *      Author: Geoffrey Mellar <mellar@house-of-plasma.com>
 *
 * Copyright (C) 2023 Geoffrey Mellar <mellar@house-of-plasma.com>
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

#ifndef INCLUDE_ECS_DATABASE_MARIADB_MARIADB_HPP_
#define INCLUDE_ECS_DATABASE_MARIADB_MARIADB_HPP_

#include <cstring>
#include <string>
#include <algorithm>
#include <utility>
#include <iterator>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <ecs/database/mariadb/MariaDB.hpp>
#include <ecs/database/types.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Blob.hpp>
#include <ecs/config.hpp>
#include <ecs/memory.hpp>
#include <ecs/database/Migrator.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <mariadb/mysql.h>

namespace ecs {
namespace db3 {

class MariaDBStatement;

class MariaDBConnection : public ConnectionImpl {
public:
	class ConnectionWrapper {
		friend class MariaDBStatement;
		friend class MariaDBConnection;
	public:
		std::string errorMessage;

		ConnectionWrapper(const ConnectionParameters &parameters);
		virtual ~ConnectionWrapper();

	private:
		ConnectionParameters   parameters;
		std::shared_ptr<MYSQL> connection;
		std::mutex             connectionMutex;
	};

	MariaDBConnection();

	virtual ~MariaDBConnection();

	std::string getPluginVersion();

	std::string getPluginDescription();

	std::string getPluginAuthor();

	std::string getPluginName();

	StatementImpl::ptr_T prepare(const std::string &query);

	ecs::db3::MigratorImpl* getMigrator(DbConnection *connection);

	bool connect(const ConnectionParameters &parameters) final override;

	bool disconnect() final override;

private:
	std::shared_ptr<ConnectionWrapper>  connection;
	static std::mutex                   libraryInitMutex;
	static bool                         libraryInit;
};

class MariaDBStatement : public ecs::db3::StatementImpl {
public:
	MariaDBStatement(const std::shared_ptr<MariaDBConnection::ConnectionWrapper> &,
			const std::string &query);
	virtual ~MariaDBStatement();

	int execute(Table *table) final override;
	bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n) final override;
	void reset() final override;
	void clearBindings() final override;
	Row::uniquePtr_T fetch() final override;
protected:
	std::shared_ptr<MariaDBConnection::ConnectionWrapper>    connection;
	std::shared_ptr<MYSQL_STMT>                              statement;
	std::string                                              query;
	std::vector<std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>>> params;
	std::shared_ptr<MYSQL_RES>                               metaResult;
};

}
}

#endif
