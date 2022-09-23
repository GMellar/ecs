/*
 * postgresql.hpp
 *
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2020 Geoffrey Mellar <mellar@gamma-kappa.com>
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

#ifndef INCLUDE_ECS_DATABASE_POSTGRESQL_POSTGRESQL_HPP_
#define INCLUDE_ECS_DATABASE_POSTGRESQL_POSTGRESQL_HPP_

#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/types.hpp>
#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>
#include <vector>
#include <list>
#include <string>
#include <ecs/UUID.hpp>
#include <algorithm>
#include <memory>
#include <cstdlib>

/** @addtogroup ecsdb
 * @{
 */

namespace ecs {
namespace db3 {


class PostgresqlStatement : public StatementImpl {
public:
	static void PGresultDeleter(PGresult *obj);

	PostgresqlStatement(PGconn *connection, const std::string &query);

	virtual ~PostgresqlStatement();

	Row::uniquePtr_T fetch();

	virtual int execute(Table *resultTable);

	virtual bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n);

	/** This one has no effect because
	 * we prepare the statement everythime.
	 */
	virtual void reset();

	virtual void clearBindings();

protected:
	std::unique_ptr<PGresult, decltype(&PGresultDeleter)> result;

	/** Connection context. Never end this connection
	 * from inside the statement because the connection
	 * is managed by the connection class.
	 */
	PGconn                              *connection;
	/** All query parameters in order of appearance. */
	std::list<ecs::db3::types::cell_T*>  bindings;
	/** Full query string */
	std::string                          query;

	int iRow;
};

class PostresqlConnection : public ConnectionImpl {
public:
	PostresqlConnection();

	virtual ~PostresqlConnection();

	std::string getPluginVersion();

	std::string getPluginDescription();

	std::string getPluginAuthor();

	std::string getPluginName();

	StatementImpl::ptr_T prepare(const std::string &query);

	ecs::db3::MigratorImpl* getMigrator(DbConnection *connection);

	bool connect(const ConnectionParameters &parameters);

	bool disconnect();

protected:
	PGconn *connection;
};

}
}

/** @} */

#endif /* INCLUDE_ECS_DATABASE_POSTGRESQL_POSTGRESQL_HPP_ */
