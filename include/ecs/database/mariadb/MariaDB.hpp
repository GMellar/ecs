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
#include <variant>
#include <ecs/database/mariadb/MariaDB.hpp>
#include <ecs/database/types.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Blob.hpp>
#include <ecs/config.hpp>
#include <ecs/memory.hpp>
#include <ecs/database/Migrator.hpp>
#include <ecs/database/BlobSource.hpp>
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
		unsigned int           connectionTimout;
		std::thread::id        threadId;
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

	bool execute(const std::string &query) final override;

	void startTransation() final override;
	void commitTransaction() final override;
	void rollbackTransaction() final override;
	void autocommit(bool) final override;

private:
	std::shared_ptr<ConnectionWrapper>  connection;
	static std::mutex                   libraryInitMutex;
	static bool                         libraryInit;
};

class MariaDBStatement : public ecs::db3::StatementImpl {
	friend class MariaDBConnection;
public:
	struct BindWrapper {
		MYSQL_BIND    bind;
		unsigned long length;
		my_bool       isnull;

		BindWrapper() {
			std::memset(&bind, '\0', sizeof(MYSQL_BIND));

			bind.length  = &length;
			bind.is_null = &isnull;
		}
	};

	struct ParameterBindHolder {
		std::vector<MYSQL_BIND>                               params;
		std::vector<std::unique_ptr<ecs::db3::types::cell_T>> cells;

		void reset() {
			params.clear();
			cells.clear();
		}

		int bind(MYSQL_STMT *stmt) {
			if(params.size()) {
				return mysql_stmt_bind_param(stmt, params.data());
			}else{
				return 0;
			}
		}
	};

	struct BindHolder {
		std::variant<
			std::vector<char>,
			BlobSource,
			std::int64_t,
			std::uint64_t,
			float,
			double> values;
		MYSQL_BIND    *binding;
		unsigned long  length;
		my_bool        isnull;

		/** Functor for getting the actual cell valu which will be
		 * stored inside the row result set.
		 */
		std::function<std::unique_ptr<ecs::db3::types::cell_T>(BindHolder*)> cellFactory;

		BindHolder(MYSQL_BIND *binding) : binding(binding), length(0), isnull(0) {

		}

		std::unique_ptr<ecs::db3::types::cell_T> getCell() {
			return cellFactory ? cellFactory(this) : std::unique_ptr<ecs::db3::types::cell_T>();
		}

		/* Resize the underlying buffer when the buffer
		 * supports resizing.
		 */
		void resize(std::size_t n) {
			if(std::vector<char> *val = std::get_if<std::vector<char>>(&values)) {
				val->resize(n);
				binding->buffer        = val->data();
				binding->buffer_length = val->size();
			}else if(BlobSource *val = std::get_if<BlobSource>(&values)) {
				val->blobData->resize(n);
				binding->buffer        = val->data();
				binding->buffer_length = val->size();
			}
		}

		void reset() {
			binding->buffer        = nullptr;
			binding->buffer_length = 0;
		}
	};

	MariaDBStatement(const std::shared_ptr<MariaDBConnection::ConnectionWrapper> &,
			const std::string &query);
	virtual ~MariaDBStatement();

	int execute(Table *table) final override;
	bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n) final override;
	void reset() final override;
	void clearBindings() final override;
	Row::uniquePtr_T fetch() final override;
	void bindBlob(const std::shared_ptr<std::basic_streambuf<char>> &,
			std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>> &);
	void bindBlob(const std::shared_ptr<std::basic_istream<char>> &,
			std::pair<MYSQL_BIND, std::unique_ptr<ecs::db3::types::cell_T>> &);
protected:
	std::shared_ptr<MariaDBConnection::ConnectionWrapper>    connection;
	std::shared_ptr<MYSQL_STMT>                              statement;
	std::string                                              query;
	ParameterBindHolder                                      parameterBindings;
	std::shared_ptr<MYSQL_RES>                               metaResult;
	/** Memory for mariadb result bindings. They need to be in a
	 * continuous chunk of data for the library.
	 */
	std::vector<MYSQL_BIND>                                  resultBindings;
	/** Bind holder which holds the cell and the variant for
	 * different data types.
	 */
	std::vector<std::unique_ptr<BindHolder>>                 resultValues;
};

}
}

#endif
