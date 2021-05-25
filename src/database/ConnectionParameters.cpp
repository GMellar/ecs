/*
 * ConnectionParameters.cpp
 *
 *  Created on: 14.12.2014
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

#include <ecs/config.hpp>
#include <ecs/database/ConnectionParameters.hpp>
#include <ecs/database/Connector.hpp>
#include <iostream>

namespace ecs {
namespace db3 {

class ConnectionParametersImpl {
public:
	ConnectionParametersImpl(const ConnectionParametersImpl &other) = default;

	ConnectionParametersImpl() {
		port            = -1;
		pluginDirectory = ECS_DATABASE_PLUGINDIR;
		pluginExtension = ECS_DATABASE_PLUGIN_EXTENSION;
	}

	virtual ~ConnectionParametersImpl() {

	}

	std::string hostname;
	std::string dbname;
	std::string user;
	std::string password;
	std::string filename;
	std::string backend;
	std::string pluginDirectory;
	std::string pluginExtension;
	int port;
};

}
}

using namespace ecs::db3;

ConnectionParameters::ConnectionParameters(){
	impl = new ConnectionParametersImpl();
}


ecs::db3::ConnectionParameters::ConnectionParameters(
		const ConnectionParameters& other) {
	impl = new ConnectionParametersImpl(*other.impl);
}

ecs::db3::ConnectionParameters::ConnectionParameters(
		ConnectionParameters&& other) {
	this->impl = new ConnectionParametersImpl(*other.impl);
}

ConnectionParameters& ecs::db3::ConnectionParameters::operator =(
		const ConnectionParameters& other) {
	*(this->impl) = *other.impl;
	return *this;
}

ConnectionParameters& ecs::db3::ConnectionParameters::operator =(
		ConnectionParameters&& other) {
	*(this->impl) = *other.impl;
	return *this;
}

ecs::db3::ConnectionParameters::~ConnectionParameters() {
	delete impl;
}


const std::string& ConnectionParameters::getDbName() const {
	return impl->dbname;
}

void ConnectionParameters::setDbName(const std::string& dbname) {
	impl->dbname = dbname;
}

const std::string& ConnectionParameters::getDbFilename() const {
	return impl->filename;
}

void ConnectionParameters::setDbFilename(const std::string& filename) {
	impl->filename = filename;
}

const std::string& ConnectionParameters::getHostname() const {
	return impl->hostname;
}

void ConnectionParameters::setHostname(const std::string& hostname) {
	impl->hostname = hostname;
}

void ConnectionParameters::setDbPassword(const std::string& password) {
	impl->password = password;
}

int ConnectionParameters::getPort() const {
	return impl->port;
}

void ConnectionParameters::setPort(int port) {
	impl->port = port;
}

const std::string& ConnectionParameters::getDbUser() const {
	return impl->user;
}

void ConnectionParameters::setDbUser(const std::string& user) {
	impl->user = user;
}

void ConnectionParameters::erasePassword() {
	/* Secure overwrite password */
	for(size_t i = 0;i < impl->password.size();i++){
		impl->password[i] = '0';
	}
	impl->password.clear();
}

const std::string& ConnectionParameters::getBackend() const {
	return impl->backend;
}

const std::string& ConnectionParameters::getFilename() const {
	return impl->filename;
}

const std::string& ConnectionParameters::getPassword() const {
	return impl->password;
}

const std::string& ConnectionParameters::getUser() const {
	return impl->user;
}

void ConnectionParameters::setBackend(const std::string& dbBackend){
	impl->backend = dbBackend;
}

const std::string &ConnectionParameters::getPluginDirectory() const {
	return impl->pluginDirectory;
}

void ConnectionParameters::setPluginDirectory ( const std::string &dir ) {
	impl->pluginDirectory = dir;
}

const std::string &ConnectionParameters::getPluginExtension() const {
	return impl->pluginExtension;
}

void ConnectionParameters::setPluginExtension ( const std::string &ext ) {
	impl->pluginExtension = ext;
}

std::unique_ptr<DbConnection> ConnectionParameters::connectPtr() {
	PluginLoader loader;
	return std::unique_ptr<DbConnection>(loader.loadPtr(*this));
}
