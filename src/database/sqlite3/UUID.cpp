/*
 * UUID.cpp
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

/* Add your header comment here */
#include <ecs/config.hpp>
#include <boost/uuid/uuid.hpp>            
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>
#include <ecs/database/sqlite3/sqlite3ext.h>
#include <mutex>

#ifdef SQLITE_CORE
/* Extension name */
#define EXTENSION_ENTRY_POINT SQLITE3_UUID_EXT
#else
#define EXTENSION_ENTRY_POINT sqlite3_extension_init
#endif

#include <ecs/database/sqlite3/sqlite3ext.h> /* Do not use <sqlite3.h>! */
SQLITE_EXTENSION_INIT1

/** @addtogroup ecsdb 
 * @{
 */

/* Insert your extension code here */
static void UUID4(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	static boost::uuids::random_generator generator;
	static std::mutex                     generatorMutex;
	
	std::lock_guard<std::mutex> lock(generatorMutex);
	boost::uuids::uuid generatedUUID = generator();
	std::string UUIDString = boost::uuids::to_string(generatedUUID);
	
	if (argc == 0) {
		sqlite3_result_text(context, UUIDString.c_str(), -1, SQLITE_TRANSIENT);
		return;
	}
	sqlite3_result_null(context);
}

static void UUID_NIL(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	static boost::uuids::nil_generator generator;
	static std::mutex                  generatorMutex;
	
	std::lock_guard<std::mutex> lock(generatorMutex);
	boost::uuids::uuid generatedUUID = generator();
	std::string UUIDString = boost::uuids::to_string(generatedUUID);
	
	if (argc == 0) {
		sqlite3_result_text(context, UUIDString.c_str(), -1, SQLITE_TRANSIENT);
		return;
	}
	sqlite3_result_null(context);
}
/* End extension code */

extern "C" ECS_EXPORT int EXTENSION_ENTRY_POINT(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);

	sqlite3_create_function_v2(db, "UUID4", -1, 
		SQLITE_UTF8, NULL, &UUID4, NULL, NULL, NULL);
	sqlite3_create_function_v2(db, "uuid_generate_v4", -1, 
		SQLITE_UTF8, NULL, &UUID4, NULL, NULL, NULL);
	sqlite3_create_function_v2(db, "uuid_nil", -1, 
		SQLITE_UTF8, NULL, &UUID_NIL, NULL, NULL, NULL);	
  return rc;
}

/** @} */
