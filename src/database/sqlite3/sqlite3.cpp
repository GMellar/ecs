/*
 * sqlite3.cpp
 *
 *  Created on: 13.12.2014
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

#include <cstring>
#include <string>
#include <utility>
#include <iterator>
#include <iostream>
#include <memory>
#include <thread>
#include <ecs/database/sqlite3/sqlite3.h>
#include <ecs/database/types.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/Blob.hpp>
#include <ecs/config.hpp>
#include <ecs/memory.hpp>
#include <ecs/database/Migrator.hpp>

/** @addtogroup ecsdb 
 * @{
 */

using namespace ecs::db3;

extern "C" ECS_EXPORT int SQLITE3_UUID_EXT(sqlite3 *,char **,const sqlite3_api_routines *);
extern "C" ECS_EXPORT int SQLITE3_UTC_EXT(sqlite3 *,char **,const sqlite3_api_routines *);

class Sqlite3Statement;

class Sqlite3Blob : public BlobBuffer {
public:
	Sqlite3Blob(void *blob, int dataBytes) : dataBytes(dataBytes), blobData(new char[dataBytes]) {
		/* Sqlite 3 blob result is only valid for the moment so there is no other way 
		 * than copying the data.
		 */
		std::memcpy(reinterpret_cast<void*>(blobData.get()), blob, dataBytes);
		setp(nullptr, nullptr);
		setg(blobData.get(), blobData.get(), blobData.get() + dataBytes);
	}
	
	virtual ~Sqlite3Blob(){
		
	}
	
	/** At the moment we use a single buffer from sqlite here so 
	 * when gptr reaches the end this is the end of the blob.
	 */
	int_type underflow(){
		return traits_type::eof();	
	}
protected:
	int                            dataBytes;
	std::unique_ptr<char[]>        blobData;
};

static void sqliteStatementDeleter(sqlite3_stmt *stmt) {
	auto rc = sqlite3_finalize(stmt);
}

class Sqlite3Statement : public StatementImpl {
public:
	Sqlite3Statement(sqlite3 *connection, const std::string &query) : sqlite3Con(connection), sqlite3Stmt(nullptr, &sqliteStatementDeleter) {
		sqlite3_stmt *stmt = nullptr;

		if(sqlite3_prepare_v2(sqlite3Con, query.c_str(), -1, &stmt, NULL) == SQLITE_OK){
			sqlite3Stmt.reset(stmt);
		}else{
			throw std::runtime_error("Statement creation failed");
		}
	}

	virtual ~Sqlite3Statement(){

	}

	int getStatus() const {
		return status;	
	}
	
	int execute(Table *dbResultTable){
		using namespace ecs::tools;
		using namespace types;

		bool        rc          = false;
		std::size_t busycounter = 1000;
		
		if (!sqlite3Stmt){
			setErrorString("There is no sqlite statement available");
			return -1;
		}
		
		/* Execute the query */
		while(1){
			/* Do a step to the next statement */
			status = sqlite3_step(sqlite3Stmt.get());
			
			/* Schema has changed which means the execution was 
			 * successful.
			 */
			if(status == SQLITE_SCHEMA){
				setErrorString(sqlite3_errmsg(sqlite3Con));
				rc = true;
				break;
			}
			
			if(status == SQLITE_ABORT){
				setErrorString(sqlite3_errmsg(sqlite3Con));
				rc = true;
				break;
			}
			
			/* Everything went good */
			if(status == SQLITE_OK || status == SQLITE_DONE){
				rc = true;
				
				/* Check if there are result rows and set names */
				int columnCount = sqlite3_column_count(sqlite3Stmt.get());
				for(int i = 0;i < columnCount;++i) {
						/* Set column name */
						dbResultTable->columnNames.push_back(sqlite3_column_name(sqlite3Stmt.get(), i));
				}
				
				break;
			}

			/* Handle sqlite library misuse error */
			if (status == SQLITE_MISUSE) {
				setErrorString(sqlite3_errmsg(sqlite3Con));
				break;
			}

			/* Constraint failed */
			if(status == SQLITE_CONSTRAINT){
				setErrorString(sqlite3_errmsg(sqlite3Con));
				break;
			}

			if (status == SQLITE_BUSY) {
				/** @todo handle busycounter */
				busycounter--;
				if(busycounter){
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				setErrorString(sqlite3_errmsg(sqlite3Con));
				break;
			}

			if (status == SQLITE_ERROR) {
				status = -1;
				setErrorString(sqlite3_errmsg(sqlite3Con));
				break;
			}

			/* We have received a row result */
			if (status == SQLITE_ROW) {
				/* Get the number of columns */
				int columns = sqlite3_column_count(sqlite3Stmt.get());
				
				/* Allocate a new row */
				auto row = ecs::make_unique<Row>();
				
				/* Iterate as los as there are columns */
				for(int i = 0;i < columns;++i){

					/* Get the column type */
					switch (sqlite3_column_type(sqlite3Stmt.get(), i)) {
						case SQLITE3_TEXT:
							*row << any::make<String>(reinterpret_cast<const char*>(sqlite3_column_text(sqlite3Stmt.get(),i)));
							break;
						case SQLITE_INTEGER:
							*row << any::make<Int64>(static_cast<int64_t>(sqlite3_column_int64(sqlite3Stmt.get(),i)));
							break;
						case SQLITE_FLOAT:
							*row << any::make<Double>(sqlite3_column_double(sqlite3Stmt.get(),i));
							break;
						case SQLITE_NULL:
							*row << new cell_T(nullptr, Null());
							break;
						case SQLITE_BLOB:
							*row << any::make<Blob>(
								std::make_shared<Sqlite3Blob>(
									static_cast<char*>(const_cast<void*>(sqlite3_column_blob(sqlite3Stmt.get(), i))),
									sqlite3_column_bytes(sqlite3Stmt.get(), i)
								)
							);
							break;
						default:
							break;
					}
				}

				/* Put the row inside the table */
				*dbResultTable << row;
				continue;
			}

			/* No return status matches so we exit the loop.
			 * This means there was an unknown error we cannot
			 * handle at this time.
			 */
			setErrorString("No matching return status " + std::to_string(status));
			break;
		}
		
		return rc == true ? 0 : -1;
	}
	
	std::int64_t lastInsertId() {
		return sqlite3_last_insert_rowid(sqlite3Con);
	}

	static void destroyBLOBArray(void *data){
		delete[] static_cast<char*>(data);
	}
	
	static void bindBLOB(sqlite3_stmt *stmt, int n, std::shared_ptr<std::basic_streambuf<char>> &streambuffer){
		/* Sqlite only allows insert of all data at once 
		* so we need to create a large array which contains all 
		* data.
		*/
		
		/* This is the stream where the data is in. We use the buffer which the user gave us. */
		std::istream        blobStream(streambuffer.get());
		/* We need the size of the buffer so go to the end */
		blobStream.seekg(0, blobStream.end);
		/* Get the size */
		auto                size = blobStream.tellg();
		/* Allocate memory. Sqlite wants a block of memory which could be very memory 
		 * intensive. There is no other way at the moment than copying.
		 */
		char               *blobVector = new char[size];
		
		/* Got to beginning */
		blobStream.seekg(0);
		
		/* Copy contents to the memory block in ram */
		std::copy(std::istreambuf_iterator<char>(blobStream), std::istreambuf_iterator<char>(), blobVector);
		
		/* Bind memory block to blob. Data is cleared inside the destruction function. */
		sqlite3_bind_blob(stmt, n, blobVector, size, &destroyBLOBArray);
	}
	
	bool bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n){
		using namespace types;
		using namespace ecs::tools;

		int status = SQLITE_ERROR;
		switch(parameter->getTypeId()){
			case types::typeId::INT64:
				status = sqlite3_bind_int64(sqlite3Stmt.get(),n+1,any::cast_reference<Int64>(*parameter));
				break;
			case types::typeId::UINT64:
				status = sqlite3_bind_int(sqlite3Stmt.get(),n+1,any::cast_reference<Uint64>(*parameter));
				break;
			case types::typeId::STRING:
				status = sqlite3_bind_text(sqlite3Stmt.get(),n+1,any::cast_reference<String>(*parameter).c_str(),-1,SQLITE_TRANSIENT);
				break;
			case types::typeId::DOUBLE:
				status = sqlite3_bind_double(sqlite3Stmt.get(),n+1,any::cast_reference<Double>(*parameter));
				break;
			case types::typeId::NIL:
				status = sqlite3_bind_null(sqlite3Stmt.get(), n+1);
				break;
			case types::typeId::BLOB:
				bindBLOB(sqlite3Stmt.get(), n+1, any::cast_reference<Blob>(*parameter));
				break;
			default:
				break;
		}

		if(status == SQLITE_OK) {
			return true;
		}
		
		return false;
	}

	void reset(){
		sqlite3_reset(sqlite3Stmt.get());
	}

	void clearBindings(){
		
	}

protected:
	int                           status;
	/** The connection is not managed by this class so never
	 * destroy the pointer on destruction. Every statement keeps
	 * the connection inside itself so the connection
	 * is alive until there is no more statement alive.
	 */
	sqlite3                      *sqlite3Con;
	std::unique_ptr<sqlite3_stmt, decltype(&sqliteStatementDeleter)> sqlite3Stmt;
};

class Sqlite3Connection : public ConnectionImpl {
public:
	Sqlite3Connection() : sqlite3Con(nullptr) {

	}

	virtual ~Sqlite3Connection(){
		disconnect();
	}

	std::string getPluginVersion() {
		return "0.0.0";
	}

	std::string getPluginDescription() {
		return "Sqlite3 database plugin";
	}

	std::string getPluginAuthor() {
		return "Geoffrey Mellar <mellar@gamma-kappa.com>";
	}

	std::string getPluginName() {
		return "database.sqlite3";
	}

	StatementImpl::ptr_T prepare(const std::string &query){
		try {
			Sqlite3Statement::uniquePtr_T result(new Sqlite3Statement(sqlite3Con, query));
			return result.release();
		}catch(...){
			setErrorMessage(sqlite3_errmsg(sqlite3Con));
			return nullptr;
		}

	}

	bool connect(const ConnectionParameters &parameters){
		int status;

		if(sqlite3Con != nullptr){
			return false;
		}

		status = sqlite3_open_v2(parameters.getDbFilename().c_str(), &sqlite3Con,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE, NULL);

		if(status != SQLITE_OK){
			return false;
		}

		status = sqlite3_exec(sqlite3Con,"PRAGMA foreign_keys = ON;",NULL,NULL,NULL);
		if(status != SQLITE_OK){
			disconnect();
			return false;
		}

		status = sqlite3_exec(sqlite3Con,"PRAGMA secure_delete=0;",NULL,NULL,NULL);
		if(status != SQLITE_OK){
			disconnect();
			return false;
		}

		status = sqlite3_exec(sqlite3Con,"PRAGMA synchronous=OFF;",NULL,NULL,NULL);
		if(status != SQLITE_OK){
			disconnect();
			return false;
		}

		status = sqlite3_exec(sqlite3Con,"PRAGMA journal_mode=WAL;",NULL,NULL,NULL);
		if(status != SQLITE_OK){
			disconnect();
			return false;
		}
		
		/* Register extension functions */
		SQLITE3_UTC_EXT(sqlite3Con,NULL,NULL);
		SQLITE3_UUID_EXT(sqlite3Con,NULL,NULL);
		return true;
	}

	bool disconnect(){
		int status;
		if(sqlite3Con == nullptr){
			return false;
		}

		status = sqlite3_close_v2(sqlite3Con);
		sqlite3Con = nullptr;

		return true;
	}

protected:
	/** This holds a shared pointer to a sqlite3
	 * connection.
	 */
	sqlite3 *sqlite3Con;
};

/** @} */

DYNLIB_BEGIN_CLASS_DEFINITION()
	DYNLIB_CLASS_DEFINITION("DatabaseConnection", Sqlite3Connection);
DYNLIB_END_CLASS_DEFINITION()

