/*
 * Row.hpp
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

#ifndef DATABASEROW_HPP_
#define DATABASEROW_HPP_

#include <ecs/config.hpp>
#include <vector>
#include <memory>
#include <cstddef>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/types.hpp>

namespace ecs {
namespace db3 {

class Result;

/** @addtogroup ecsdb
 * @{
 */

class ECS_EXPORT RowBase {
public:
	POINTER_DEFINITIONS(RowBase);

	RowBase();
	virtual ~RowBase();

	virtual void clear() = 0;

	virtual RowBase &operator<<(ecs::db3::types::cell_T::ptr_T ptr) = 0;

	virtual RowBase &operator<<(ecs::db3::types::cell_T::uniquePtr_T &ptr) = 0;

	virtual RowBase &operator<<(ecs::db3::types::cell_T::uniquePtr_T &&ptr) = 0;

	virtual std::vector<ecs::db3::types::cell_T::uniquePtr_T>::size_type size() const = 0;

	virtual ecs::db3::types::cell_T &operator[](int columnNumber) = 0;

	virtual ecs::db3::types::cell_T &at(int columnNumber) = 0;

protected:

};

class ECS_EXPORT Row : public RowBase {
public:
	POINTER_DEFINITIONS(Row);

	Row();

	Row(Row &&other) = default;

	virtual ~Row();

	virtual void clear();

	virtual RowBase &operator<<(ecs::db3::types::cell_T::ptr_T ptr);
	
	virtual RowBase &operator<<(ecs::db3::types::cell_T::uniquePtr_T &ptr);
	
	virtual RowBase &operator<<(ecs::db3::types::cell_T::uniquePtr_T &&ptr);
	
	virtual std::vector<ecs::db3::types::cell_T::uniquePtr_T>::size_type size() const;
	
	virtual ecs::db3::types::cell_T &operator[](int columnNumber);

	virtual ecs::db3::types::cell_T &at(int columnNumber);

	std::vector<ecs::db3::types::cell_T::uniquePtr_T> data;
private:

};

/** Wrapper class for the row result.
 *
 */
class RowResult {
	friend class Result;
public:
	RowResult(RowBase::uniquePtr_T impl);

	RowResult(const RowResult &other) = delete;

	RowResult(RowResult &&other) = default;

	virtual ~RowResult();

	RowResult& operator=(const RowResult &other) = delete;

	RowResult& operator=(RowResult &&other) = default;

	std::vector<ecs::db3::types::cell_T::uniquePtr_T>::size_type size() const;

	ecs::db3::types::cell_T &operator[](int columnNumber);

	ecs::db3::types::cell_T &at(int columnNumber);

	operator bool() const;

private:
	RowResult();

	RowBase::uniquePtr_T impl;
};

Row::ptr_T createRow();

/** @} */

}
}

#endif /* DATABASEROW_HPP_ */
