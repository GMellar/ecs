from ecspy import *

# Set some basic parameters for sqlite database
params = ConnectionParameters()
params.setDbFilename("test.sqlite")
params.setBackend("sqlite3")

# Connect the database (Creates sqlite file if not available)
connection = params.connect()

# Create a migrator object and initialize schema
migrator = Migrator(connection)
migrator.initSchema();

def migration_0_1(connection):
	stmt = connection.prepare("CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, a VARCHAR NOT NULL, b INTEGER NOT NULL, c DOUBLE NOT NULL);")
	# We must return the result here to tell if the migration was 
	# succesful
	return stmt.execute()

# Create a first migration for version 1
migrator.addMigration(0, 1, migration_0_1)
migrator.startMigration()
