/*
 * Connection.cpp
 *
 *  Created on: 06.12.2014
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

#include <ecs/database/Connection.hpp>
#include "impl/ConnectionImpl.cpp"
#include "impl/DbConnectionImpl.cpp"
#include <ecs/database/impl/StatementInternals.hpp>
#include <fstream>
#include <algorithm>
#include <exception>
#include <ecs/database/Exception.hpp>

using namespace ecs::db3;

ecs::db3::DbConnection::DbConnection() : impl(new DbConnectionImpl){
	
}

ecs::db3::DbConnection::~DbConnection() {
	delete impl;
}

Statement::ptr_T ecs::db3::DbConnection::preparePtr(
		const std::string& query) {
	/* Check if we are still connected */
	if(!impl) {
		throw exceptions::Exception("No implementation present");
	}

	/* Use the implementation provided function here to build the statement */
	StatementImpl::sharedPtr_T statementImplementation(impl->module->prepare(query));
	Statement::uniquePtr_T     statement(new Statement(this));
	
	/* We check that here because both, the statement class and 
	 * the statement implementation must be valid. 
	 */
	if(statementImplementation && statement){
		statement->impl->stmt = std::move(statementImplementation);

		/* Release the pointer ownership */
		return statement.release();
	}
	
	/* Most database libraries will fail when creating a statement 
	 * which has invalid syntax. To capture that situation, an error is thrown 
	 * because the statement is not able to do anything useful. 
	 */
	throw exceptions::Exception("Statement creation failed");
}

bool DbConnection::execute ( const std::string &query ) {
	auto statement = prepare(query);
	if(!statement){
		throw exceptions::Exception("Query execution failed: " + query + " " + statement->getErrorMessage());
	}else{
		return statement->execute();
	}
}


Statement::ptr_T ecs::db3::DbConnection::prepareFromFilePtr(const std::string &filename){
	std::ifstream stream(filename.c_str());
	
	if(stream.is_open()){
		std::string query(std::istreambuf_iterator<char>(stream), {});
		return preparePtr(query);
	}
	
	return nullptr;
}

const ecs::db3::ConnectionParameters &ecs::db3::DbConnection::getParameters() const {
	return impl->parameters;
}
