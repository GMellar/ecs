/*
 * StatementImpl.hpp
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
#ifndef SRC_ECSDB_DB3_IMPL_STATEMENTIMPL_HPP_
#define SRC_ECSDB_DB3_IMPL_STATEMENTIMPL_HPP_

#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <ecs/config.hpp>
#include <ecs/database/QueryResult.hpp>
#include <ecs/database/Statement.hpp>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb 
 * @{
 */

/** This is the statement implementation which must be implemented in plugins. There are
 * some things to consider implementing this class. The first thing is never throw inside this
 * implemented class because we use loadable modules.
 */
class ECS_EXPORT StatementImpl : public std::enable_shared_from_this<StatementImpl> {
public:
	POINTER_DEFINITIONS(StatementImpl);

	StatementImpl();

	StatementImpl(const StatementImpl &statement) = default;

	StatementImpl(StatementImpl &&statement) = default;

	StatementImpl &operator=(const StatementImpl &statement) = default;

	StatementImpl &operator=(StatementImpl &&statement) = default;

	virtual ~StatementImpl();

	/** Implement this to return the result table and an error
	 * indicator. Returning 0 means no error. The table pointer can be filled with
	 * values and rows.
	 */
	virtual int execute(Table *table) = 0;

	/** Bind a value to the statement. You muist not own the given memory. This is owned in another
	 * class. You can assume that the bind value is valid until this class exists.
	 */
	virtual bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n) = 0;

	virtual std::int64_t lastInsertId();

	virtual void reset() = 0;

	virtual void clearBindings() = 0;

	const std::string& getErrorString() const;

	virtual int getStatus() const {return 0;};
	
	void setErrorString(const std::string& dbErrorString);

	virtual Row::uniquePtr_T fetch() = 0;
protected:
	/** Error string for last operation */
	std::string dbErrorString;
};

/** @} */

}
}

#endif /* SRC_ECSDB_DB3_IMPL_STATEMENTIMPL_HPP_ */
