include(GenerateExportHeader)

find_package(Threads)

option(ECS_SQLITE3_DRIVER "Enable sqlite database driver" TRUE)

if(NOT ${ECS_SQLITE3_DRIVER})
	set(sqlite3_dbplugin_obj)
	return()
endif()

##########################################################
# Add final plugin library
##########################################################
file(GLOB sqlite3_sources
	"${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/UUID.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/UTC.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c")

add_library(sqlite3_dbplugin_obj OBJECT ${sqlite3_sources})
target_compile_definitions(sqlite3_dbplugin_obj 
	PRIVATE 
		DYNLIB_NO_PLUGIN)
target_compile_definitions(sqlite3_dbplugin_obj PRIVATE
SQLITE_ENABLE_MATH_FUNCTIONS
SQLITE_ENABLE_FTS3 
SQLITE_ENABLE_FTS3_PARENTHESIS 
# For ordinary FTS3/FTS4 queries, the search tree depth is approximately the base-2 logarithm of the number of terms in the right-hand side of the MATCH operator. However, for phrase queries and NEAR queries the search tree depth is linear in the number of right-hand side terms. So the default depth limit of 12 is sufficient for up to 4095 ordinary terms on a MATCH, it is only sufficient for 11 or 12 phrase or NEAR terms. Even so, the default is more than enough for most application. 
SQLITE_FTS3_MAX_EXPR_DEPTH=16
SQLITE_ENABLE_RTREE 
SQLITE_ENABLE_JSON1 
#SQLITE_OMIT_LOAD_EXTENSION 
# This macro determines the initial setting for PRAGMA automatic_index for newly opened database connections. For all versions of SQLite through 3.7.17, automatic indices are normally enabled for new database connections if this compile-time option is omitted. However, that might change in future releases of SQLite. 
SQLITE_DEFAULT_AUTOMATIC_INDEX=1 
# This macro determines if SQLite creates databases with the auto_vacuum flag set by default to OFF (0), FULL (1), or INCREMENTAL (2). The default value is 0 meaning that databases are created with auto-vacuum turned off. In any case the compile-time default may be overridden by the PRAGMA auto_vacuum command. 
SQLITE_DEFAULT_AUTOVACUUM=2 
# This macro sets the default size of the page-cache for each attached database, in pages. This can be overridden by the PRAGMA cache_size command. The default value is 2000. 
SQLITE_DEFAULT_CACHE_SIZE=4096 
# The schema format number for a new database can be set at runtime using the PRAGMA legacy_file_format command. 
SQLITE_DEFAULT_FILE_FORMAT=4 
# The default numeric file permissions for newly created database files under unix. If not specified, the default is 0644 which means that the files is globally readable but only writable by the creator. 
SQLITE_DEFAULT_FILE_PERMISSIONS=0644 
# This macro determines whether enforcement of foreign key constraints is enabled or disabled by default for new database connections. Each database connection can always turn enforcement of foreign key constraints on and off and run-time using the foreign_keys pragma. Enforcement of foreign key constraints is normally off by default, but if this compile-time parameter is set to 1, enforcement of foreign key constraints will be on by default. 
SQLITE_DEFAULT_FOREIGN_KEYS=1 
# This macro is used to set the default page-size used when a database is created. The value assigned must be a power of 2. The default value is 1024. The compile-time default may be overridden at runtime by the PRAGMA page_size command. 
SQLITE_DEFAULT_PAGE_SIZE=2048 
# This macro is used to set the default page-size used when a database is created. The value assigned must be a power of 2. The default value is 1024. The compile-time default may be overridden at runtime by the PRAGMA page_size command. 
SQLITE_DEFAULT_WAL_AUTOCHECKPOINT=1000 
# To put it another way, SQLITE_THREADSAFE=1 sets the default threading mode to Serialized. SQLITE_THREADSAFE=2 sets the default threading mode to Multi-threaded. And SQLITE_THREADSAFE=0 sets the threading mode to Single-threaded.
SQLITE_THREADSAFE=3 
# This option controls whether temporary files are stored on disk or in memory. The meanings for various settings of this compile-time option are as follows:
# SQLITE_TEMP_STORE	Meaning
#   0	Always use temporary files
#   1	Use files by default but allow the PRAGMA temp_store command to override
#   2	Use memory by default but allow the PRAGMA temp_store command to override
#   3	Always use memory
SQLITE_TEMP_STORE=2 
# This option causes the URI filename process logic to be enabled by default. 
SQLITE_USE_URI 
SQLITE_ALLOW_URI_AUTHORITY 
# When defined, this C-preprocessor macro activates extra code that attempts to detect misuse of the SQLite API, such as passing in NULL pointers to required parameters or using objects after they have been destroyed. 
SQLITE_ENABLE_API_ARMOR 
# When this C-preprocessor macro is defined, SQLite includes some additional APIs that provide convenient access to meta-data about tables and queries. The APIs that are enabled by this option are:
SQLITE_ENABLE_COLUMN_METADATA 
# This option enables the dbstat virtual table. 
SQLITE_ENABLE_DBSTAT_VTAB 
# This option adds extra logic to SQLite that allows it to release unused memory upon request. This option must be enabled in order for the sqlite3_release_memory() interface to work. If this compile-time option is not used, the sqlite3_release_memory() interface is a no-op. 
SQLITE_ENABLE_MEMORY_MANAGEMENT
# This option enables the sqlite3_serialize() and sqlite3_deserialize() interfaces.
# Future releases of SQLite might enable those interfaces by default and instead offer an SQLITE_OMIT_DESERIALIZE option to leave them out.
SQLITE_ENABLE_DESERIALIZE
# Don't compile loadable extension module but register them
SQLITE_CORE
)

generate_export_header(sqlite3_dbplugin_obj)
target_include_directories(sqlite3_dbplugin_obj 
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
# This is needed because we always need the headers from the library
target_include_directories(sqlite3_dbplugin_obj 
	PRIVATE 
		$<TARGET_PROPERTY:ecs,INTERFACE_INCLUDE_DIRECTORIES>)

install(TARGETS sqlite3_dbplugin_obj
	EXPORT "${PROJECT_NAME}Targets"
)

##########################################################
# Link sqlite3 plugin with pthread when available
##########################################################

if(HAVE_PTHREAD)
	target_link_libraries(sqlite3_dbplugin_obj 
		PRIVATE 
			Threads::Threads)
endif()

# Create runtime loadable module
add_library(sqlite3_dbplugin MODULE ${sqlite3_sources})
target_compile_definitions(sqlite3_dbplugin PRIVATE
SQLITE_ENABLE_MATH_FUNCTIONS
SQLITE_ENABLE_FTS3 
SQLITE_ENABLE_FTS3_PARENTHESIS 
# For ordinary FTS3/FTS4 queries, the search tree depth is approximately the base-2 logarithm of the number of terms in the right-hand side of the MATCH operator. However, for phrase queries and NEAR queries the search tree depth is linear in the number of right-hand side terms. So the default depth limit of 12 is sufficient for up to 4095 ordinary terms on a MATCH, it is only sufficient for 11 or 12 phrase or NEAR terms. Even so, the default is more than enough for most application. 
SQLITE_FTS3_MAX_EXPR_DEPTH=16
SQLITE_ENABLE_RTREE 
SQLITE_ENABLE_JSON1 
#SQLITE_OMIT_LOAD_EXTENSION 
# This macro determines the initial setting for PRAGMA automatic_index for newly opened database connections. For all versions of SQLite through 3.7.17, automatic indices are normally enabled for new database connections if this compile-time option is omitted. However, that might change in future releases of SQLite. 
SQLITE_DEFAULT_AUTOMATIC_INDEX=1 
# This macro determines if SQLite creates databases with the auto_vacuum flag set by default to OFF (0), FULL (1), or INCREMENTAL (2). The default value is 0 meaning that databases are created with auto-vacuum turned off. In any case the compile-time default may be overridden by the PRAGMA auto_vacuum command. 
SQLITE_DEFAULT_AUTOVACUUM=2 
# This macro sets the default size of the page-cache for each attached database, in pages. This can be overridden by the PRAGMA cache_size command. The default value is 2000. 
SQLITE_DEFAULT_CACHE_SIZE=4096 
# The schema format number for a new database can be set at runtime using the PRAGMA legacy_file_format command. 
SQLITE_DEFAULT_FILE_FORMAT=4 
# The default numeric file permissions for newly created database files under unix. If not specified, the default is 0644 which means that the files is globally readable but only writable by the creator. 
SQLITE_DEFAULT_FILE_PERMISSIONS=0644 
# This macro determines whether enforcement of foreign key constraints is enabled or disabled by default for new database connections. Each database connection can always turn enforcement of foreign key constraints on and off and run-time using the foreign_keys pragma. Enforcement of foreign key constraints is normally off by default, but if this compile-time parameter is set to 1, enforcement of foreign key constraints will be on by default. 
SQLITE_DEFAULT_FOREIGN_KEYS=1 
# This macro is used to set the default page-size used when a database is created. The value assigned must be a power of 2. The default value is 1024. The compile-time default may be overridden at runtime by the PRAGMA page_size command. 
SQLITE_DEFAULT_PAGE_SIZE=2048 
# This macro is used to set the default page-size used when a database is created. The value assigned must be a power of 2. The default value is 1024. The compile-time default may be overridden at runtime by the PRAGMA page_size command. 
SQLITE_DEFAULT_WAL_AUTOCHECKPOINT=1000 
# To put it another way, SQLITE_THREADSAFE=1 sets the default threading mode to Serialized. SQLITE_THREADSAFE=2 sets the default threading mode to Multi-threaded. And SQLITE_THREADSAFE=0 sets the threading mode to Single-threaded.
SQLITE_THREADSAFE=3 
# This option controls whether temporary files are stored on disk or in memory. The meanings for various settings of this compile-time option are as follows:
# SQLITE_TEMP_STORE	Meaning
#   0	Always use temporary files
#   1	Use files by default but allow the PRAGMA temp_store command to override
#   2	Use memory by default but allow the PRAGMA temp_store command to override
#   3	Always use memory
SQLITE_TEMP_STORE=2 
# This option causes the URI filename process logic to be enabled by default. 
SQLITE_USE_URI 
SQLITE_ALLOW_URI_AUTHORITY 
# When defined, this C-preprocessor macro activates extra code that attempts to detect misuse of the SQLite API, such as passing in NULL pointers to required parameters or using objects after they have been destroyed. 
SQLITE_ENABLE_API_ARMOR 
# When this C-preprocessor macro is defined, SQLite includes some additional APIs that provide convenient access to meta-data about tables and queries. The APIs that are enabled by this option are:
SQLITE_ENABLE_COLUMN_METADATA 
# This option enables the dbstat virtual table. 
SQLITE_ENABLE_DBSTAT_VTAB 
# This option adds extra logic to SQLite that allows it to release unused memory upon request. This option must be enabled in order for the sqlite3_release_memory() interface to work. If this compile-time option is not used, the sqlite3_release_memory() interface is a no-op. 
SQLITE_ENABLE_MEMORY_MANAGEMENT
# This option enables the sqlite3_serialize() and sqlite3_deserialize() interfaces.
# Future releases of SQLite might enable those interfaces by default and instead offer an SQLITE_OMIT_DESERIALIZE option to leave them out.
SQLITE_ENABLE_DESERIALIZE
# Don't compile loadable extension module but register them
SQLITE_CORE
)
target_link_libraries(sqlite3_dbplugin 
	PRIVATE 
		ecs
		$<TARGET_PROPERTY:sqlite3_dbplugin_obj,INTERFACE_LINK_LIBRARIES>)
set_target_properties(sqlite3_dbplugin PROPERTIES OUTPUT_NAME sqlite3)
set_target_properties(sqlite3_dbplugin PROPERTIES PREFIX "")
install(TARGETS sqlite3_dbplugin
	LIBRARY DESTINATION lib/ecs/dbplugin
)
