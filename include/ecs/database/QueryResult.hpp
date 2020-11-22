/*
 * QueryResult.hpp
 *
 *  Created on: 11.12.2014
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

#ifndef DATABASEQUERYRESULT_HPP_
#define DATABASEQUERYRESULT_HPP_

#include <ecs/config.hpp>
#include <ostream>
#include <string>
#include <ecs/database/Table.hpp>
#include <ecs/database/Row.hpp>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb
 * @{
 */

class Statement;
class DbConnection;
class ResultImpl;
class StatementInternals;

class ECS_EXPORT Result {
	friend class Statement;
public:
	Result(const Result &result) = delete;

	/** Only move semantic is allowed */
	Result(Result &&result);

	Result &operator=(const Result &result) = delete;

	Result &operator=(Result &&result) = delete;

	virtual ~Result();

	void clear();

	/** Get error message from the last executed statement. */
	const std::string &getErrorMessage() const;

	/** Get the number of result rows */
	std::size_t size() const;

	/** Return result row */
	Row &operator[](std::size_t n);

	/** This will return true if there 
	 * is a result. 
	 */
	operator bool() const;
	
	/** Print result as JSON to a stream */
	void serializeToJSON(std::ostream &stream);
	
protected:
	/** Implementation details */
	ResultImpl *impl;

	/** Construct with statement internals to
	 * keep the connection open as long as the result
	 * exists.
	 */
	Result(Statement *stmt);
};

/** @} */

}
}

#endif /* DATABASEQUERYRESULT_HPP_ */
