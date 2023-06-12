/*
 * Connection.cpp
 *
 *  Created on: 17.05.2021
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

#ifndef INCLUDE_ECS_DATABASE_SQLITE3_SQLITE_HPP_
#define INCLUDE_ECS_DATABASE_SQLITE3_SQLITE_HPP_


#include <cstring>
#include <string>
#include <algorithm>
#include <utility>
#include <iterator>
#include <iostream>
#include <memory>
#include <thread>
#include <functional>
#include <ecs/database/sqlite3/sqlite3.h>
#include <ecs/database/types.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Blob.hpp>
#include <ecs/config.hpp>
#include <ecs/memory.hpp>
#include <ecs/database/Migrator.hpp>
#include <ecs/database/BlobSource.hpp>

namespace ecs {
namespace db3 {


class Sqlite3Statement;

class Sqlite3Blob : public BlobBuffer {
public:
	Sqlite3Blob(void *blob, int dataBytes);

	virtual ~Sqlite3Blob();

	/** At the moment we use a single buffer from sqlite here so
	 * when gptr reaches the end this is the end of the blob.
	 */
	int_type underflow();
protected:
	int                            dataBytes;
	std::unique_ptr<char[]>        blobData;
};


class Sqlite3Statement : public StatementImpl {
public:
	struct FetchStrategy {
		virtual Row::uniquePtr_T fetch(Sqlite3Statement *stmt) = 0;
	};

	static void sqliteStatementDeleter(sqlite3_stmt *stmt);

	Sqlite3Statement(sqlite3 *connection, const std::string &query);
	virtual ~Sqlite3Statement();
	int getStatus() const final override;
	Row::uniquePtr_T fetch() final override;
	int step(Row::uniquePtr_T &row);
	int execute(Table *dbResultTable) final override;
	std::int64_t lastInsertId() final override;
	static void destroyBLOBArray(void *data);
	static void bindBLOB(sqlite3_stmt *stmt, int n, std::shared_ptr<std::basic_streambuf<char>> &streambuffer);
	static void bindIstream(sqlite3_stmt *stmt, int n, std::shared_ptr<std::basic_istream<char>> &streambuffer);
	bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n) final override;
	void reset() final override;
	void clearBindings() final override;

protected:
	int                           status;
	/** The connection is not managed by this class so never
	 * destroy the pointer on destruction. Every statement keeps
	 * the connection inside itself so the connection
	 * is alive until there is no more statement alive.
	 */
	sqlite3                      *sqlite3Con;
	std::unique_ptr<sqlite3_stmt, decltype(&sqliteStatementDeleter)> sqlite3Stmt;

	/** Whenever a row is returned it is stored here.
	 *
	 */
	Row::uniquePtr_T row;

	std::function<Row::uniquePtr_T(Sqlite3Statement*)> doFetchRow;
	const char *pzTail;
};

class ECS_EXPORT Sqlite3Connection : public ConnectionImpl {
public:
	Sqlite3Connection();

	virtual ~Sqlite3Connection();

	std::string getPluginVersion();

	std::string getPluginDescription();

	std::string getPluginAuthor();

	std::string getPluginName();

	StatementImpl::ptr_T prepare(const std::string &query) final override;

	bool connect(const ConnectionParameters &parameters) final override;

	bool disconnect() final override;

	ecs::db3::MigratorImpl* getMigrator(DbConnection *connection);

protected:
	/** This holds a shared pointer to a sqlite3
	 * connection.
	 */
	sqlite3 *sqlite3Con;
};

}
}


#endif /* INCLUDE_ECS_DATABASE_SQLITE3_SQLITE3_HPP_ */
