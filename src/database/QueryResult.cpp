/*
 * QueryResult.cpp
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

#include <ecs/database/QueryResult.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/Exception.hpp>
#include "impl/ResultImpl.cpp"
#include <ecs/database/impl/StatementInternals.hpp>
#include <utility>
#include <algorithm>

using namespace ecs::db3;

ecs::db3::Result::Result(std::shared_ptr<Statement> stmt) : impl(new ResultImpl(stmt)) {

}

ecs::db3::Result::Result(Result &&result) : impl(nullptr) {
	std::swap(this->impl, result.impl);
}

ecs::db3::Result::~Result() {
	clear();
	if(impl) delete impl;
}

std::string ecs::db3::Result::getErrorMessage() const {
	return impl->stmt->getErrorMessage();
}

ecs::db3::RowResult ecs::db3::Result::fetch() {
	return RowResult(impl->stmt->fetch());
}

TableResult ecs::db3::Result::fetchAll() {
	RowResult row;

	while(row = this->fetch()) {
		(*impl->resultTable) << std::move(row.impl);
	}

	return TableResult(std::move(impl->resultTable));
}

void ecs::db3::Result::clear() {
	if(impl) impl->resultTable.reset();
}

ecs::db3::Result::operator bool() const {
	return isValid();
}

Result& ecs::db3::Result::operator =(Result &result) {
	std::swap(this->impl, result.impl);
	return *this;
}

Result& ecs::db3::Result::operator =(Result &&result) {
	std::swap(this->impl, result.impl);
	return *this;
}

bool ecs::db3::Result::isValid() const {
	if(!impl) return false;

	if(!impl->resultTable){
		return false;
	}
	return true;
}
