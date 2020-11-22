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

using namespace ecs::db3;

ecs::db3::Result::Result(Statement *stmt) {
	impl = new ResultImpl(stmt->impl);
}

ecs::db3::Result::~Result() {
	clear();
	impl->stmt->stmt->reset();
	delete impl;
}

const std::string& ecs::db3::Result::getErrorMessage() const {
	return impl->stmt->stmt->getErrorString();
}

std::size_t ecs::db3::Result::size() const {
	return impl->resultTable->size();
}

Row& ecs::db3::Result::operator [](std::size_t n) {
	try {
		return (*impl->resultTable)[n];
	}catch(...){
		throw exceptions::Exception("Out of range");
	}
}

void ecs::db3::Result::clear() {
	impl->resultTable.reset();
}

ecs::db3::Result::operator bool() const {
	if(!impl->resultTable){
		return false;
	}
	return true;
}
