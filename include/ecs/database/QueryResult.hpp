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
#include <memory>
#include <iterator>
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
	class Iterator : public std::input_iterator_tag {
	public:
		using difference_type   = std::size_t;
		using value_type        = RowResult;
		using pointer           = RowResult*;
		using reference         = RowResult&;
		using iterator_category = std::input_iterator_tag;

		Iterator(Result *result, RowResult row) : result(result), row(std::move(row)) {

		}

		Iterator &operator++() {
			row = result->fetch();
			return *this;
		}

		bool operator==(const Iterator &other) const {
			return row.impl.get() == other.row.impl.get();
		}

		reference operator*() {
			return row;
		}

		reference operator->() {
			return row;
		}
	private:
		Result   *result;
		RowResult row;
	};

	using iterator       = Iterator;
	using const_iterator = const Iterator;

	Result() = delete;

	Result(const Result &result) = delete;

	/** Only move semantic is allowed */
	Result(Result &&result);

	Result &operator=(const Result &result) = delete;

	Result &operator=(Result &result);

	Result &operator=(Result &&result);

	virtual ~Result();

	/** Get error message from the last executed statement. */
	std::string getErrorMessage() const;

	/** This will return true if there 
	 * is a result. 
	 */
	operator bool() const;
	
	bool isValid() const;

	/** Fetch the next row from the result. When there is
	 * no more row an empty pointer is returned.
	 */
	RowResult fetch();

	/** Fetch all results from the executed
	 * query. This command must not be called twice
	 * because the result table is moved.
	 */
	TableResult fetchAll();
protected:
	/** Implementation details */
	ResultImpl *impl;

	/** Construct with statement internals to
	 * keep the connection open as long as the result
	 * exists.
	 */
	Result(std::shared_ptr<Statement> stmt);

	/** Clear the result table */
	void clear();
};

/** @} */

}
}

#endif /* DATABASEQUERYRESULT_HPP_ */
