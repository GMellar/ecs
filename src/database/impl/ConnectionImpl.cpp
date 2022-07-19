/*
 * ConnectionImpl.cpp
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

#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/MigratorImpl.hpp>
#include <ecs/database/Migrator.hpp>

ecs::db3::ConnectionImpl::ConnectionImpl() {

}

bool ecs::db3::ConnectionImpl::connect(const ConnectionParameters& parameters) {
	return false;
}

bool ecs::db3::ConnectionImpl::disconnect() {
	return false;
}

bool ecs::db3::ConnectionImpl::execute ( const std::string &query ) {
	std::unique_ptr<Table> table(new Table);

	/* Create the statement */
	auto statement = this->prepare(query);
	if(!statement){
		return false;
	}else{
		return statement->execute(table.get());
	}
}

std::string ecs::db3::ConnectionImpl::getErrorMessage() {
	return errorMessage;
}

void ecs::db3::ConnectionImpl::setErrorMessage ( const std::string &message ) {
	errorMessage = message;
}


ecs::db3::ConnectionImpl::~ConnectionImpl() {

}
