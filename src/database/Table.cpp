/*
 * Table.cpp
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

#include <ecs/database/Table.hpp>

using namespace ecs::db3;

ecs::db3::Table::Table() {
}

ecs::db3::Table::~Table() {
	clear();
}

void ecs::db3::Table::clear() {
	data.clear();
}

Row& ecs::db3::Table::operator <<(Row::ptr_T row) {
	data.push_back(Row::uniquePtr_T(row));
	return *row;
}

Row &Table::operator<< ( Row::uniquePtr_T &row ) {
	data.push_back(std::move(row));
	return *data.back();
}

Row& ecs::db3::Table::operator [](int rowNumber) const {
	return *data.at(rowNumber);
}

std::size_t ecs::db3::Table::size() const {
	return data.size();
}

const std::string &ecs::db3::Table::getColumnName(int n) const {
	return columnNames[n];
}

ecs::db3::TableBase::TableBase() {
}

ecs::db3::TableBase::~TableBase() {
}

Row& ecs::db3::TableBase::at(int n) const {
	return (*this)[n];
}

Row& ecs::db3::Table::at(int n) const {
	return (*this)[n];
}
