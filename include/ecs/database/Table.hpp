/*
 * Table.hpp
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

#ifndef DATABASETABLE_HPP_
#define DATABASETABLE_HPP_

#include <ecs/config.hpp>
#include <vector>
#include <map>
#include <memory>
#include <iterator>
#include <string>
#include <ecs/database/Row.hpp>
#include <ostream>
#include <ecs/PointerDefinitions.hpp>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb
 * @{
 */

class ECS_EXPORT TableBase {
public:
	POINTER_DEFINITIONS(TableBase);

	TableBase();
	virtual ~TableBase();

	virtual void clear() = 0;

	virtual RowBase &operator<<(RowBase::ptr_T row) = 0;

	virtual RowBase &operator<<(RowBase::uniquePtr_T row) = 0;

	virtual RowBase &operator[](int rowNumber) const = 0;

	virtual std::size_t size() const = 0;

	/** Get the column name for the n-th column where n starts at zero.
	 *
	 */
	virtual const std::string &getColumnName(int n) const = 0;

	virtual RowBase & at(int n) const;
};

class ECS_EXPORT Table : public TableBase {
public:
	POINTER_DEFINITIONS(Table);
	
	std::vector<RowBase::uniquePtr_T> data;
	/** Name of the column if present */
	std::vector<std::string>      columnNames;
	
	Table();

	virtual ~Table();

	virtual void clear();

	virtual RowBase &operator<<(RowBase::ptr_T row);

	virtual RowBase &operator<<(RowBase::uniquePtr_T row);
	
	virtual RowBase &operator[](int rowNumber) const;

	virtual RowBase &at(int n) const;

	virtual std::size_t size() const;
	
	virtual const std::string &getColumnName(int n) const;

};

class TableResult {
public:
	TableResult(TableBase::uniquePtr_T impl);

	TableResult(const TableResult &other) = delete;

	TableResult(TableResult &&other) = delete;

	virtual ~TableResult();

	TableResult &operator=(const TableResult &other) = delete;

	TableResult &operator=(TableResult &&other) = delete;

	RowBase &at(int n) const;

	std::size_t size() const;

	const std::string &getColumnName(int n) const;

	operator bool() const;


private:
	TableBase::uniquePtr_T impl;
};

/** @} */

}
}

#endif /* DATABASETABLE_HPP_ */
