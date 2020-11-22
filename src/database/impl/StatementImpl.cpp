/*
 * StatementImpl.cpp
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

#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Exception.hpp>

ecs::db3::StatementImpl::StatementImpl() {

}

ecs::db3::StatementImpl::~StatementImpl() {

}

const std::string& ecs::db3::StatementImpl::getErrorString() const {
	return dbErrorString;
}

void ecs::db3::StatementImpl::setErrorString(
		const std::string& dbErrorString) {
	this->dbErrorString = dbErrorString;
}

std::int64_t ecs::db3::StatementImpl::lastInsertId() {
	throw exceptions::Exception("Not Implemented");
}
