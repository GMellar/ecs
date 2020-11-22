/*
 * DatabaseInterface.cpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/database/DatabaseInterface.hpp>
#include <ecs/database/Exception.hpp>

using namespace ecs::db3;

ecs::db3::DatabaseInterfaceBase::~DatabaseInterfaceBase() {
}

DbConnection::sharedPtr_T ecs::db3::DatabaseInterfaceBase::getConnection() {
	if(!connection) {
		throw exceptions::Exception("Connection invalid");
	}
	return connection->getConnection();
}

const DbConnection::sharedPtr_T ecs::db3::DatabaseInterfaceBase::getConnection() const {
	if(!connection) {
		throw exceptions::Exception("Connection invalid");
	}
	return connection->getConnection();
}

const ConnectionParameters& ecs::db3::DatabaseInterfaceBase::getParameters() const {
	return getConnection()->getParameters();
}

ecs::db3::DatabaseInterfaceBase::DatabaseInterfaceBase() {

}

ecs::db3::DatabaseInterfaceBase::DatabaseInterfaceBase(
		DbConnection::sharedPtr_T connection) : connection(new InterfaceConnection(connection)) {

}

ecs::db3::InterfaceConnectionBase::~InterfaceConnectionBase() {

}

ecs::db3::InterfaceConnectionBase::InterfaceConnectionBase() {

}

ecs::db3::InterfaceConnection::~InterfaceConnection() {

}

ecs::db3::InterfaceConnection::InterfaceConnection(
		ConnectionParameters& params) : connection(params.connect()) {

}

ecs::db3::InterfaceConnection::InterfaceConnection(
		DbConnection::sharedPtr_T connection) : connection(connection) {

}

DbConnection::sharedPtr_T ecs::db3::InterfaceConnection::getConnection() {
	return connection;
}

void ecs::db3::DatabaseInterfaceBase::clearConnection() {
	connection.reset();
}

ecs::db3::DatabaseInterfaceBase::DatabaseInterfaceBase(
		InterfaceConnectionBase::sharedPtr_T connection) : connection(connection) {
}

ecs::db3::InterfaceConnectionBase::operator DbConnection::sharedPtr_T() {
	return getConnection();
}

ecs::db3::InterfaceConnectionPerThread::~InterfaceConnectionPerThread() {
}

ecs::db3::InterfaceConnectionPerThread::InterfaceConnectionPerThread(
		ConnectionParameters& params) : params(params) {
}

DbConnection::sharedPtr_T ecs::db3::InterfaceConnectionPerThread::getConnection() {
	thread_local static DbConnection::sharedPtr_T connection;
	if(!connection) {
		connection = params.connect();
	}

	return connection;
}
