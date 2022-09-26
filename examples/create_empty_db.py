from ecspy import *

# Set some basic parameters for sqlite database
params = ConnectionParameters()
params.setDbFilename("test.sqlite")
params.setBackend("sqlite3")

# Connect the database (Creates sqlite file if not available)
connection = params.connect()

def migration_0_1(connection):
	stmt = connection.prepare("""
		CREATE TABLE IF NOT EXISTS test (
		id INTEGER PRIMARY KEY, 
		a VARCHAR NOT NULL, 
		b INTEGER NOT NULL, 
		c DOUBLE NOT NULL, 
		d BLOB);
		""")
	# We must return the result here to tell if the migration was 
	# succesful
	return stmt.execute()

# Create a migrator object and initialize schema
migrator = Migrator(connection)
migrator.initSchema();

# Create a first migration for version 1
migrator.addMigration(0, 1, migration_0_1)
migrator.startMigration()

# Insert one entry
stmt = connection.prepare("INSERT INTO test(a,b,c,d) VALUES(?,?,?,?);")
stmt.bind("test")
stmt.bind(1000)
stmt.bind(10.45)
stmt.bindBlob("BLOB")
res = stmt.execute()

# Read one entry
stmt = connection.prepare("SELECT * FROM test LIMIT 5;")
res = stmt.execute()
value = res.fetch()

while value.isValid() == True:
	print(value.at(1).getString())
	value = res.fetch()
