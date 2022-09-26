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

RowBase& ecs::db3::Table::operator <<(RowBase::ptr_T row) {
	data.push_back(RowBase::uniquePtr_T(row));
	return *row;
}

RowBase &Table::operator<< ( RowBase::uniquePtr_T row ) {
	data.push_back(std::move(row));
	return *data.back();
}

RowBase& ecs::db3::Table::operator [](int rowNumber) const {
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

RowBase& ecs::db3::TableBase::at(int n) const {
	return (*this)[n];
}

RowBase& ecs::db3::Table::at(int n) const {
	return (*this)[n];
}

ecs::db3::TableResult::TableResult(TableBase::uniquePtr_T impl) : impl(std::move(impl)) {

}

ecs::db3::TableResult::~TableResult() {

}

RowBase& ecs::db3::TableResult::at(int n) const {
	return impl->at(n);
}

std::size_t ecs::db3::TableResult::size() const {
	return impl->size();
}

const std::string& ecs::db3::TableResult::getColumnName(int n) const {
	return impl->getColumnName(n);
}

ecs::db3::TableResult::operator bool() const {
	return impl.get() != nullptr;
}
