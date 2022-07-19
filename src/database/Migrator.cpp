/*
 * Migrator.cpp
 *
 *  Created on: 16.11.2017
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

#include <ecs/database/Migrator.hpp>
#include <ecs/memory.hpp>
#include <utility>
#include <exception>
#include <ecs/database/QueryResult.hpp>
#include <ecs/database/Statement.hpp>
#include <string>
#include <iostream>
#include <ecs/database/Exception.hpp>

// Intentionally because the following includes need this
using namespace ecs::db3;
#include "impl/MigratorImpl.cpp"

Migrator::Migrator (DbConnection::sharedPtr_T connection) : connection(connection) {

}

Migrator::~Migrator() {

}

void Migrator::addMigration (Migrator::Migration::ptr_T m) {
	Migrator::Migration::uniquePtr_T migration(m);
	addMigration(std::move(migration));
}

void Migrator::addMigration ( Migrator::Migration::uniquePtr_T m ) {
	migrations.insert(std::make_pair(m->from_version, std::move(m)));
}

Migrator::Migration::ptr_T Migrator::getMigration ( int fromVersion ) {
	auto result = migrations.find(fromVersion);
	if(result == migrations.end()){
		/* No migration for the source version found */
		return nullptr;
	}else{
		/* Found migration for this version */
		return result->second.get();
	}
}


int Migrator::startMigration() {
	Statement::sharedPtr_T    stmt;
	Migration::ptr_T          migration;
	auto migrator = connection->getMigrator();
	int migrationCounter = 0;
	
	/* Do migrations until there is no migration left */
	while((migration = getMigration(migrator->getSchemaVersion())) != nullptr) {
		/* Do migration */
		bool migrationResult = migrator->doMigration(migration);
		
		if(migrationResult == false){
			throw exceptions::Exception("Migration error");
		}
		
		/* At this point we assume that the migration was successful 
			* because no exception was thrown. 
			*/
		migrator->setSchemaVersion(migration->getToVersion());
		migrationCounter++;
	}	
	
	return migrationCounter;
}

int Migrator::initSchema() {
	auto migrator = connection->getMigrator();
	return migrator->initSchema();
}


Migrator::Migration::Migration ( int from_version, int to_version ) : from_version(from_version), to_version(to_version) {
	
}

Migrator::Migration::~Migration() {

}

int Migrator::Migration::getFromVersion() const {
	return from_version;
}

int Migrator::Migration::getToVersion() const {
	return to_version;
}


Migrator::MigrationFunction::MigrationFunction ( int from, int to, std::function<bool(DbConnection*)> fn ) : Migration ( from, to ) , fn(fn) {

}

Migrator::MigrationFunction::~MigrationFunction() {

}

bool Migrator::MigrationFunction::upMigration ( DbConnection *connection ) {
	return fn(connection);
}
