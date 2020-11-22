#include <ecs/Database.hpp>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <string>
#include <list>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ecs/TicToc.hpp>
#include <boost/filesystem.hpp>

ecs::db3::ConnectionParameters params;

bool migration1(ecs::db3::Migrator &m, ecs::db3::DbConnection *connection){
	std::cout << "Migrate database from version 0 to 1" << std::endl;
	return true;
}

bool migration2(ecs::db3::Migrator &m, ecs::db3::DbConnection *connection){
	std::cout << "Migrate database from version 1 to 2" << std::endl;
	return true;
}

TEST_CASE("Testing transactions", "[ecsdb_schema]"){
	using namespace ecs::db3;
	PluginLoader         loader;
	
	params.setPluginDirectory("build/plugins/database");
	params.setBackend("sqlite3");
	params.setDbFilename("./migrator_test.sqlite3");
	
	boost::filesystem::remove(params.getDbFilename());

	auto connection = loader.load(params);
	auto connection1 = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	/* Initialize the migrator */
	Migrator migration;
	/* Initialize schema which means adding schema info table */
	REQUIRE_NOTHROW(migration.initSchema(connection));
	
	/* Add all migrations you need */
	REQUIRE_NOTHROW(migration.addMigration(new Migrator::MigrationFunction(0, 1, &migration1)));
	REQUIRE_NOTHROW(migration.addMigration(new Migrator::MigrationFunction(1, 2, &migration2)));
	
	/* Start the migration */
	REQUIRE_NOTHROW(migration.startMigration(connection));	
}

struct InterfaceTest : public ecs::db3::DatabaseInterface<InterfaceTest> {
	POINTER_DEFINITIONS(InterfaceTest);
	friend class ecs::db3::DatabaseInterface<InterfaceTest>;

	std::int64_t id;
	int          number;
	std::string  text;

	static InterfaceTest create(ecs::db3::InterfaceConnectionBase &c,
			std::int64_t number, std::string text) {
		/* This statement creates the interface we can return.
		 * you don't need to worry about the connection because
		 * the connection is created by the interface connection class.
		 *
		 * Once called get you have a fully working interface to the database.
		 * Consider that your interface class needs to call the base class constructor
		 * to hold the connection. Otherwhise no query can be made to the database.
		 */
		auto result = get(c);
		REQUIRE_NOTHROW(result.getConnection());
		auto stmt = result.getConnection()->prepare("INSERT INTO table1(`number`, `text`) VALUES(?,?);");
		stmt->bind(number);
		stmt->bind(text);
		stmt->execute();
		return get(c, stmt->lastInsertId());
	}

	/** We must call the DatabaseInterface constructor here because we perform a query
	 * inside the constructor which needs a connection.
	 */
	InterfaceTest(ecs::db3::DbConnection::sharedPtr_T con, std::int64_t id) : DatabaseInterface(con), id(id) {
		auto stmt = con->prepare("SELECT `id`,`number`,`text` FROM table1 WHERE `id`=?;");
		stmt->bind(id);
		auto result = stmt->execute();
		number = result[0][1].cast_reference<std::int64_t>();
		text   = result[0][2].cast_reference<std::string>();
	}

protected:
	InterfaceTest(ecs::db3::DbConnection::sharedPtr_T con) : DatabaseInterface(con) {

	}

private:
	InterfaceTest() {

	}
};

TEST_CASE("Testing Interfaces", "[ecsdb_interface]") {
	using namespace ecs::db3;

	params.setDbFilename("./interface_test.sqlite");
	boost::filesystem::remove(params.getDbFilename());

	auto connection = params.connect();
	connection->execute("DROP TABLE IF EXISTS table1;");
	connection->execute("CREATE TABLE table1 ("
			"`id` INTEGER PRIMARY KEY,"
			"`number` INTEGER NOT NULL,"
			"`text` VARCHAR NOT NULL);");

	{
		InterfaceConnection db(params);
		auto result = InterfaceTest::create(db, 123456, "This is a test");
		REQUIRE(result.number == 123456);
		REQUIRE(result.text == "This is a test");
	}
}

TEST_CASE( "Testing opening a connection", "[ecsdb]" ) {
	using namespace ecs::db3;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	connection->execute("DROP TABLE IF EXISTS documents;");
	
	auto statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS documents ("
		"document_id VARCHAR PRIMARY KEY DEFAULT (uuid_generate_v4()),"
		"file_ending VARCHAR NOT NULL,"
		"filename VARCHAR NOT NULL,"
		"date_added VARCHAR NOT NULL DEFAULT (utc_timestamp()),"
		"document_date VARCHAR NOT NULL DEFAULT (utc_timestamp()),"
		"folder VARCHAR NOT NULL DEFAULT '/'"
		");"
	);
	auto result = statement->execute();
	REQUIRE(result == true);
	
	statement = connection->prepare("INSERT INTO documents(filename) VALUES(?);");
	statement->bind("test.pdf");
	REQUIRE_THROWS(statement->execute());
}

TEST_CASE( "Testing opening a non existing plugin", "[ecsdb]" ) {
	using namespace ecs::db3;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite");
	params.setDbFilename("./test.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	DbConnection::sharedPtr_T connection;
	REQUIRE_THROWS(connection = loader.load(params));
}

TEST_CASE( "Test column names", "[ecsdb]" ) {
	using namespace ecs::db3;
	ecs::tools::TicToc t;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test1.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	t.tic("Loading connection");
	auto connection = loader.load(params);
	t.toc();
	REQUIRE( connection.get() != nullptr );
	
	t.tic("Statement creation");
	auto statement = connection->prepare("DROP TABLE IF EXISTS testtable;");
	t.toc();
	statement->execute();
	
	t.tic("Create testtable");
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 INT PRIMARY KEY DEFAULT (uuid_generate_v4()),"
		"col2 DOUBLE NOT NULL,"
		"col3 VARCHAR DEFAULT NULL,"
		"col4 VARCHAR NOT NULL DEFAULT (utc_timestamp())"
		");"
	);
	statement->execute();
	t.toc();
	
	t.tic("Insert in testtable with uuid and timestamp");
	statement = connection->prepare("INSERT INTO testtable(col2,col3) VALUES(?, ?);");
	statement->bind(12.5);
	statement->bind("Test");
	statement->execute();
	t.toc();
	
	t.tic("Insert in testtable with uuid and timestamp");
	statement = connection->prepare("INSERT INTO testtable(col2,col3) VALUES(?, ?);");
	statement->bind(12.5);
	statement->bind("Test");
	statement->execute();
	t.toc();
	
	t.tic("Insert in testtable with uuid and timestamp");
	statement = connection->prepare("INSERT INTO testtable(col2,col3) VALUES(?, ?);");
	statement->bind(12.5);
	statement->bind("Test");
	statement->execute();
	t.toc();
	
	t.tic("Selecting inserted values");
	statement = connection->prepare("SELECT * FROM testtable;");
	auto result = statement->execute();
	REQUIRE(result.size() == 3);
	t.toc();
	std::cout << std::endl;
}

TEST_CASE( "Testing a lot of inserts and queries", "[ecsdb]" ) {
	using namespace ecs::db3;
	ecs::tools::TicToc t;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename(":memory:");
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );

	auto statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 INT PRIMARY KEY,"
		"col2 DOUBLE NOT NULL"
		");"
	);
	statement->execute();
	
	statement = connection->prepare("DROP TABLE testtable;");
	statement->execute();
	
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 INT PRIMARY KEY,"
		"col2 DOUBLE NOT NULL"
		");"
	);
	statement->execute();
	
	connection->execute("BEGIN TRANSACTION;");
	
	statement = connection->prepare("INSERT INTO testtable(col1,col2) VALUES(?, ?);");
	t.tic("Inserting 10000 rows");
	for(int64_t i = 0;i < 10000;++i){
		statement->bind(i);
		statement->bind(12.6);
		statement->execute();
		statement->reset();
	}
	t.toc();
	connection->execute("END TRANSACTION;");
	
	t.tic("Selecting 10000 rows");
	statement = connection->prepare("SELECT * FROM testtable;");
	auto res = statement->execute();
	for(int i = 0;i < 10000;++i){
		REQUIRE(res[i][0].cast_reference<int64_t>() == i);	
	}
	t.toc();
	REQUIRE(res.size() == 10000);
}

TEST_CASE( "Testing a lot of inserts with auto generated uuid values", "[ecsdb]" ) {
	using namespace    ecs::db3;
	ecs::tools::TicToc t;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test2.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	auto statement = connection->prepare("DROP TABLE IF EXISTS testtable;");
	statement->execute();
	
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 VARCHAR PRIMARY KEY DEFAULT (uuid_generate_v4()),"
		"col2 INT"
		");"
	);
	statement->execute();
	
	t.tic("Inserting 1000 rows with default uuid value");
	statement = connection->prepare("INSERT INTO testtable(col2) VALUES(?);");
	for(int64_t i = 0;i < 1000;++i){
		statement->bind(i);
		statement->execute();
		statement->reset();
	}
	t.toc();
}

TEST_CASE("Testing if throwing works when executing invalid SQL statements", "[ecsdb]") {
	using namespace ecs::db3;
	PluginLoader         loader;
	params.setBackend("sqlite3");
	params.setDbFilename("./test5.sqlite3");
	boost::filesystem::remove(params.getDbFilename());

	auto connection = loader.load(params);
	Statement::sharedPtr_T statement;
	
	try{
		statement = connection->prepare("VERY INVALID STATEMENT;");
		statement->execute();
	}catch(std::exception &e){
		
	}
	REQUIRE_THROWS(connection->prepare("jasdajslkdjalksdjal"));
	REQUIRE_NOTHROW(connection->prepare("CREATE TABLE testtable(col2 INT);"));
}

TEST_CASE( "Testing a lot of inserts with auto generated timestamp values", "[ecsdb]" ) {
	using namespace ecs::db3;
	ecs::tools::TicToc t;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test2.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	auto statement = connection->prepare("DROP TABLE IF EXISTS testtable;");
	statement->execute();
	
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 VARCHAR PRIMARY KEY DEFAULT (uuid_generate_v4()),"
		"col2 INT"
		");"
	);
	statement->execute();
	
	t.tic("Inserting 1000 rows with default timestamp value");
	statement = connection->prepare("INSERT INTO testtable(col2) VALUES(?);");
	for(int64_t i = 0;i < 1000;++i){
		statement->bind(i);
		statement->execute();
		statement->reset();
	}
	t.toc();
}

TEST_CASE( "Testing a lot of inserts with auto generated timestamp values and statement creation sinside loop", "[ecsdb]" ) {
	using namespace ecs::db3;
	ecs::tools::TicToc t;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test2.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	auto statement = connection->prepare("DROP TABLE IF EXISTS testtable;");
	statement->execute();
	
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 VARCHAR PRIMARY KEY DEFAULT (utc_timestamp()),"
		"col2 INT"
		");"
	);
	statement->execute();
	
	t.tic("Inserting 1000 rows with default timestamp value and statement creation inside loop");
	for(int64_t i = 0;i < 1000;++i){
		statement = connection->prepare("INSERT INTO testtable(col1, col2) VALUES(?,?);");
		statement->bind(std::to_string(i));
		statement->bind(i);
		statement->execute();
	}
	t.toc();
}

TEST_CASE("Test for blobs", "[ecsdb_blob]"){
	using namespace ecs::db3;
	
	PluginLoader         loader;
	
	params.setBackend("sqlite3");
	params.setDbFilename("./test3.sqlite3");
	boost::filesystem::remove(params.getDbFilename());
	
	auto connection = loader.load(params);
	REQUIRE( connection.get() != nullptr );
	
	auto statement = connection->prepare("DROP TABLE IF EXISTS testtable;");
	statement->execute();
	
	statement = connection->prepare(
		"CREATE TABLE IF NOT EXISTS testtable ("
		"col1 VARCHAR PRIMARY KEY DEFAULT (utc_timestamp()),"
		"col2 BLOB"
		");"
	);
	statement->execute();
	
	
	std::ifstream file("Makefile");
	statement = connection->prepare("INSERT INTO testtable(col2,col1) VALUES(?,?);");
	statement->bind(file.rdbuf());
	statement->bind("test");
	statement->execute();
	file.close();
	
	std::ofstream outFile("blob.test");
	statement = connection->prepare("SELECT col2 FROM testtable WHERE col1 = ?;");
	statement->bind("test");
	auto result = statement->execute();
	REQUIRE(result == true);
	REQUIRE(result.size() == 1);
	outFile << result[0][0].cast_reference<types::Blob::type>();
	outFile.close();
	
}

TEST_CASE("Migration test", "[ecsdb_migration]") {
	
}
