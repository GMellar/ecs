/*
 * StatementInternals.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/database/impl/StatementInternals.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>

ecs::db3::StatementInternals::StatementInternals(DbConnection *connection)  {
	this->connection = connection->impl->clone();
}

ecs::db3::StatementInternals::StatementInternals(DbConnectionImpl* connection) {
	this->connection = connection->clone();
}

ecs::db3::StatementInternals::~StatementInternals() {
	delete connection;
}

ecs::db3::StatementInternals* ecs::db3::StatementInternals::clone() {
	std::unique_ptr<StatementInternals> result(new StatementInternals(connection));
	result->stmt       = this->stmt;
	return result.release();
}
