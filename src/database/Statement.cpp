/*
 * Statement.cpp
 *
 *  Created on: 23.02.2016
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

#include <ecs/database/Statement.hpp>
#include "impl/StatementImpl.cpp"
#include "impl/StatementInternals.cpp"
#include <ecs/database/impl/ResultImpl.hpp>
#include <ecs/memory.hpp>
#include <functional>
#include <exception>
#include <ecs/database/Exception.hpp>


ecs::db3::Statement::Statement(DbConnection *connection) {
	impl = new StatementInternals(connection);
}

ecs::db3::Statement::~Statement() {
	delete impl;
}

void ecs::db3::Statement::reset() {
	impl->stmt->reset();
	impl->bindings.clear();
}

std::string ecs::db3::Statement::getErrorMessage() const {
	return impl->stmt->getErrorString();
}


ecs::db3::Result ecs::db3::Statement::execute() {
	/* Create the new table which is then passed to
	 * the module.
	 */
	auto resultTable = std::make_unique<Table>();

	/* Result class constructed from the statement internals */
	Result result(shared_from_this());

	/* Execute SQL statement */
	auto rc = impl->stmt->execute(resultTable.get());
	
	/* Get the result table. This may be empty. */
	result.impl->resultTable = std::move(resultTable);
	
	/* Check if execution was successful. If not then throw */
	if(rc != 0){
		throw exceptions::Exception(
			"Return value:  " + std::to_string(rc) + "\n"
			"Error message: " + impl->stmt->getErrorString());
	}
	
	return result;
}

std::unique_ptr<ecs::db3::Result> ecs::db3::Statement::executePtr() {
	return std::make_unique<ecs::db3::Result>(std::move(execute()));
}

void ecs::db3::Statement::clearBindings() {
	impl->bindings.clear();
}

bool ecs::db3::Statement::bind(ecs::db3::types::cell_T::ptr_T ptr) {
	auto binding = ecs::db3::types::cell_T::uniquePtr_T(ptr);
	bool rc = impl->stmt->bind(ptr, nullptr, impl->bindings.size());
	impl->bindings.push_back(std::move(binding));
	return rc;
}

bool ecs::db3::Statement::bind(ecs::db3::types::cell_T::uniquePtr_T &&ptr) {
	bool rc = impl->stmt->bind(ptr.get(), nullptr, impl->bindings.size());
	impl->bindings.push_back(std::move(ptr));
	return rc;
}

bool ecs::db3::Statement::bind(std::int64_t value) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Int64>(value));
}

bool ecs::db3::Statement::bind(std::uint64_t value) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Uint64>(value));
}

bool ecs::db3::Statement::bind(const std::string& value) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::String>(value));
}

bool ecs::db3::Statement::bind(double value) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Double>(value));
}

bool ecs::db3::Statement::bind(std::nullptr_t) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Null>(nullptr));	
}

bool ecs::db3::Statement::bind() {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Null>(nullptr));
}

bool ecs::db3::Statement::bind(float value) {
	using namespace ecs::db3::types;
	return bind(ecs::tools::any::make<types::Float>(value));
}

bool ecs::db3::Statement::bind(
		std::unique_ptr<std::basic_istream<char> > stream) {

	using namespace ecs::db3::types;

	if(!stream){
		throw exceptions::Exception("Binding a nullptr streambuffer to a statement is not allowed");
	}
	std::shared_ptr<std::basic_istream<char>> s(stream.release());
	return bind(ecs::tools::any::make<types::BlobInput>(s));
}

bool ecs::db3::Statement::bind(
		const std::shared_ptr<std::basic_streambuf<char> > &stream) {

	using namespace ecs::db3::types;

	if(!stream){
		throw exceptions::Exception("Binding a nullptr streambuffer to a statement is not allowed");
	}

	return bind(ecs::tools::any::make<types::Blob>(stream));
}

std::int64_t ecs::db3::Statement::lastInsertId() {
	return impl->stmt->lastInsertId();
}

ecs::db3::Row::uniquePtr_T ecs::db3::Statement::fetch() {
	return impl->stmt->fetch();
}
