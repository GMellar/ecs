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

#include <ecs/database/sqlite3/sqlite3.hpp>
#include "MigratorImplSqlite3.cpp"
#include <boost/iostreams/stream_buffer.hpp>

/** @addtogroup ecsdb 
 * @{
 */

using namespace ecs::db3;

extern "C" ECS_EXPORT int SQLITE3_UUID_EXT(sqlite3 *,char **,const sqlite3_api_routines *);
extern "C" ECS_EXPORT int SQLITE3_UTC_EXT(sqlite3 *,char **,const sqlite3_api_routines *);

void Sqlite3Statement::sqliteStatementDeleter(sqlite3_stmt *stmt) {
	auto rc = sqlite3_finalize(stmt);
}

Sqlite3Blob::Sqlite3Blob(void *blob, int dataBytes) : dataBytes(dataBytes), blobData(new char[dataBytes]) {
	/* Sqlite 3 blob result is only valid for the moment so there is no other way
	 * than copying the data.
	 */
	std::memcpy(reinterpret_cast<void*>(blobData.get()), blob, dataBytes);
	setp(nullptr, nullptr);
	setg(blobData.get(), blobData.get(), blobData.get() + dataBytes);
}

Sqlite3Blob::~Sqlite3Blob() {
	
}

/** At the moment we use a single buffer from sqlite here so
 * when gptr reaches the end this is the end of the blob.
 */
BlobBuffer::int_type Sqlite3Blob::underflow(){
	return traits_type::eof();
}


Sqlite3Statement::Sqlite3Statement(sqlite3 *connection, const std::string &query) : sqlite3Con(connection),
		sqlite3Stmt(nullptr, &sqliteStatementDeleter), status(0) {
	sqlite3_stmt *stmt = nullptr;
	auto res = sqlite3_prepare_v2(sqlite3Con, query.c_str(), -1, &stmt, &pzTail);
	if(res == SQLITE_OK){
		sqlite3Stmt.reset(stmt);
	}else{
		throw std::runtime_error("Statement creation failed: " + std::to_string(res));
	}

	doFetchRow = [](Sqlite3Statement *stmt){
		return std::unique_ptr<Row>();
	};
}

Sqlite3Statement::~Sqlite3Statement(){

}

int Sqlite3Statement::getStatus() const {
	return status;
}

Row::uniquePtr_T Sqlite3Statement::fetch() {
	using namespace ecs::tools;
	using namespace types;

	return doFetchRow(this);
}

int Sqlite3Statement::step(Row::uniquePtr_T &row) {
	using namespace ecs::tools;
	using namespace types;

	bool        rc          = false;
	std::size_t busycounter = 1000;

	while(1) {
		status = sqlite3_step(sqlite3Stmt.get());

		if(status == SQLITE_SCHEMA){
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if(status == SQLITE_ABORT){
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if(status == SQLITE_OK || status == SQLITE_DONE){
			return 0;
		}else if (status == SQLITE_MISUSE) {
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if(status == SQLITE_CONSTRAINT){
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if (status == SQLITE_BUSY) {
			busycounter--;
			if(busycounter){
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if (status == SQLITE_ERROR) {
			status = -1;
			setErrorString(sqlite3_errmsg(sqlite3Con));
			return -1;
		}else if (status == SQLITE_ROW) {
			row = std::make_unique<Row>();
			std::shared_ptr<boost::iostreams::stream_buffer<BlobSource>> blobBuffer;

			/* Get the number of columns */
			int columns = sqlite3_column_count(sqlite3Stmt.get());

			/* Iterate as los as there are columns */
			for(int i = 0;i < columns;++i){

				/* Get the column type */
				switch (sqlite3_column_type(sqlite3Stmt.get(), i)) {
					case SQLITE3_TEXT:
						*row << any::make<String>(reinterpret_cast<const char*>(sqlite3_column_text(sqlite3Stmt.get(),i)), 
							sqlite3_column_bytes(sqlite3Stmt.get(),i)
						);
						break;
					case SQLITE_INTEGER:
						*row << any::make<Int64>(static_cast<int64_t>(sqlite3_column_int64(sqlite3Stmt.get(),i)));
						break;
					case SQLITE_FLOAT:
						*row << any::make<Double>(sqlite3_column_double(sqlite3Stmt.get(),i));
						break;
					case SQLITE_NULL:
						*row << std::make_unique<cell_T>(nullptr, Null());
						break;
					case SQLITE_BLOB:
						blobBuffer = std::make_shared<boost::iostreams::stream_buffer<BlobSource>>(
								static_cast<char*>(const_cast<void*>(sqlite3_column_blob(sqlite3Stmt.get(), i))),
								sqlite3_column_bytes(sqlite3Stmt.get(), i)
						);

						*row << any::make<Blob>(blobBuffer);
						break;
					default:
						row.reset();
						setErrorString("Unsupported row result");
						return -1;
						break;
				}
			}

			return 1;
		}else{
			/* No return status matches so we exit the loop.
			 * This means there was an unknown error we cannot
			 * handle at this time.
			 */
			setErrorString("No matching return status " + std::to_string(status));
			return -1;
		}
	}
}

int Sqlite3Statement::execute(Table *dbResultTable){
	using namespace ecs::tools;
	using namespace types;

	if (!sqlite3Stmt){
		setErrorString("There is no sqlite statement available");
		return -1;
	}
	
	row.reset();
	auto rc = step(row);
	if(rc < 0) {
		doFetchRow = [](Sqlite3Statement *stmt){
			return std::unique_ptr<Row>();
		};

	}else if(rc == 0) {
		/* No more values expected */
		rc = 0;
		doFetchRow = [](Sqlite3Statement *stmt){
			return std::unique_ptr<Row>();
		};
	}else if(rc == 1) {
		/* More values expected which need a fetch */
		rc = 0;
		doFetchRow = [](Sqlite3Statement *stmt){
			stmt->doFetchRow = [](Sqlite3Statement *stmt){
				std::unique_ptr<Row> row;
				stmt->step(row);
				return row;
			};

			return std::move(stmt->row);
		};
	}else{
		doFetchRow = [](Sqlite3Statement *stmt){
			return std::unique_ptr<Row>();
		};
		/* Unexpected result code */
		rc = -1;
	}

	/* Check if there are result rows and set names */
	int columnCount = sqlite3_column_count(sqlite3Stmt.get());
	for(int i = 0;i < columnCount;++i) {
			/* Set column name */
			dbResultTable->columnNames.push_back(sqlite3_column_name(sqlite3Stmt.get(), i));
	}

	return rc;
}

std::int64_t Sqlite3Statement::lastInsertId() {
	return sqlite3_last_insert_rowid(sqlite3Con);
}

void Sqlite3Statement::destroyBLOBArray(void *data){
	delete[] static_cast<char*>(data);
}

void ecs::db3::Sqlite3Statement::bindIstream(sqlite3_stmt *stmt, int n,
		std::shared_ptr<std::basic_istream<char> > &blobStream) {
	/* Sqlite only allows insert of all data at once
	* so we need to create a large array which contains all
	* data.
	*/

	/* We need the size of the buffer so go to the end */
	blobStream->seekg(0, std::ios_base::end);

	/* Get the size */
	auto                size = blobStream->tellg();
	/* Allocate memory. Sqlite wants a block of memory which could be very memory
	 * intensive. There is no other way at the moment than copying.
	 */
	char               *blobVector = new char[size];

	/* Got to beginning */
	blobStream->seekg(0, std::ios_base::beg);

	/* Copy contents to the memory block in ram */
	std::copy(std::istreambuf_iterator<char>(*blobStream), std::istreambuf_iterator<char>(), blobVector);

	/* Bind memory block to blob. Data is cleared inside the destruction function. */
	sqlite3_bind_blob(stmt, n, blobVector, size, &destroyBLOBArray);
}

void Sqlite3Statement::bindBLOB(sqlite3_stmt *stmt, int n, std::shared_ptr<std::basic_streambuf<char>> &streambuffer){
	/* Sqlite only allows insert of all data at once
	* so we need to create a large array which contains all
	* data.
	*/

	/* This is the stream where the data is in. We use the buffer which the user gave us. */
	std::istream        blobStream(streambuffer.get());

	/* We need the size of the buffer so go to the end */
	blobStream.seekg(0, std::ios_base::end);

	/* Get the size */
	auto                size = blobStream.tellg();
	/* Allocate memory. Sqlite wants a block of memory which could be very memory
	 * intensive. There is no other way at the moment than copying.
	 */
	char               *blobVector = new char[size];

	/* Got to beginning */
	blobStream.seekg(0, std::ios_base::beg);
	
	/* Copy contents to the memory block in ram */
	std::copy(std::istreambuf_iterator<char>(blobStream), std::istreambuf_iterator<char>(), blobVector);
	
	/* Bind memory block to blob. Data is cleared inside the destruction function. */
	sqlite3_bind_blob(stmt, n, blobVector, size, &destroyBLOBArray);
}

bool Sqlite3Statement::bind(ecs::db3::types::cell_T *parameter, const std::string *parameterName, int n){
	using namespace types;
	using namespace ecs::tools;

	int status = SQLITE_ERROR;
	switch(parameter->getTypeId()){
		case types::typeId::int64_T:
			status = sqlite3_bind_int64(sqlite3Stmt.get(),n+1,any::cast_reference<Int64>(*parameter));
			break;
		case types::typeId::uint64_T:
			status = sqlite3_bind_int(sqlite3Stmt.get(),n+1,any::cast_reference<Uint64>(*parameter));
			break;
		case types::typeId::string:
			status = sqlite3_bind_text(sqlite3Stmt.get(),n+1,any::cast_reference<String>(*parameter).c_str(),-1,SQLITE_TRANSIENT);
			break;
		case types::typeId::double_T:
			status = sqlite3_bind_double(sqlite3Stmt.get(),n+1,any::cast_reference<Double>(*parameter));
			break;
		case types::typeId::float_T:
			status = sqlite3_bind_double(sqlite3Stmt.get(),n+1,any::cast_reference<Float>(*parameter));
			break;
		case types::typeId::null:
			status = sqlite3_bind_null(sqlite3Stmt.get(), n+1);
			break;
		case types::typeId::blob:
			bindBLOB(sqlite3Stmt.get(), n+1, any::cast_reference<Blob>(*parameter));
			break;
		case types::typeId::blobInput:
			bindIstream(sqlite3Stmt.get(), n+1, any::cast_reference<BlobInput>(*parameter));
			break;
		case types::typeId::boolean_T:
			status = sqlite3_bind_int(sqlite3Stmt.get(),n+1,any::cast_reference<Boolean>(*parameter) == true ? 1 : 0);
			break;
		default:
			break;
	}

	if(status == SQLITE_OK) {
		return true;
	}

	return false;
}

void Sqlite3Statement::reset(){
	sqlite3_reset(sqlite3Stmt.get());
}

void Sqlite3Statement::clearBindings(){

}


Sqlite3Connection::Sqlite3Connection() : sqlite3Con(nullptr) {

}

Sqlite3Connection::~Sqlite3Connection(){
	disconnect();
}

std::string Sqlite3Connection::getPluginVersion() {
	return "0.0.0";
}

std::string Sqlite3Connection::getPluginDescription() {
	return "Sqlite3 database plugin";
}

std::string Sqlite3Connection::getPluginAuthor() {
	return "Geoffrey Mellar <mellar@gamma-kappa.com>";
}

std::string Sqlite3Connection::getPluginName() {
	return "database.sqlite3";
}

StatementImpl::ptr_T Sqlite3Connection::prepare(const std::string &query){
	try {
		auto result = std::make_unique<Sqlite3Statement>(sqlite3Con, query);
		return result.release();
	}catch(...){
		setErrorMessage(sqlite3_errmsg(sqlite3Con));
		return nullptr;
	}

}

bool Sqlite3Connection::connect(const ConnectionParameters &parameters){
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
	SQLITE3_UTC_EXT(sqlite3Con, NULL, NULL);
	SQLITE3_UUID_EXT(sqlite3Con, NULL, NULL);
	return true;
}

bool Sqlite3Connection::disconnect(){
	int status;
	if(sqlite3Con == nullptr){
		return false;
	}

	status = sqlite3_close_v2(sqlite3Con);
	sqlite3Con = nullptr;

	return true;
}

ecs::db3::MigratorImpl* Sqlite3Connection::getMigrator(DbConnection *connection) {
	auto result = std::make_unique<ecs::db3::MigratorImplSqlite3>(connection);
	return result.release();
}

/** @} */

DYNLIB_BEGIN_CLASS_DEFINITION()
	DYNLIB_CLASS_DEFINITION("DatabaseConnection", ecs::db3::Sqlite3Connection);
DYNLIB_END_CLASS_DEFINITION()
