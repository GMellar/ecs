# includes
find_path(MariaDBClient_INCLUDE_DIR
        NAMES mysql.h
        PATH_SUFFIXES mariadb mysql
        )

# library
set(BAK_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
find_library(MariaDBClient_LIBRARY
        NAMES mariadb libmariadb mariadbclient libmariadbclient mysqlclient libmysqlclient
        PATH_SUFFIXES mariadb mysql
        )
set(CMAKE_FIND_LIBRARY_SUFFIXES ${BAK_CMAKE_FIND_LIBRARY_SUFFIXES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MariaDBClient DEFAULT_MSG MariaDBClient_LIBRARY MariaDBClient_INCLUDE_DIR)

if(MariaDBClient_FOUND)
    if(NOT TARGET MariaDBClient::MariaDBClient)
        add_library(MariaDBClient::MariaDBClient UNKNOWN IMPORTED)
        set_target_properties(MariaDBClient::MariaDBClient PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MariaDBClient_INCLUDE_DIR}"
            IMPORTED_LOCATION "${MariaDBClient_LIBRARY}")
    endif()
endif()

option(ECS_MARIADB_DRIVER "Enable MariaDB database driver" TRUE)

if(NOT TARGET MariaDBClient::MariaDBClient)
	message(WARNING "Did not find MariaDB client libraries")
	return()
endif()

if(TARGET MariaDBClient::MariaDBClient AND ${ECS_MARIADB_DRIVER})
	add_library(mariadb_dbplugin_obj OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/MariaDB.cpp")
	
	target_compile_definitions(mariadb_dbplugin_obj 
		PRIVATE DYNLIB_NO_PLUGIN)
	
	target_include_directories(mariadb_dbplugin_obj 
		PRIVATE 
			$<TARGET_PROPERTY:ecs,INTERFACE_INCLUDE_DIRECTORIES>
	)
	
	target_link_libraries(mariadb_dbplugin_obj 
		PRIVATE
			MariaDBClient::MariaDBClient
		INTERFACE
			MariaDBClient::MariaDBClient
	)
	
	install(TARGETS mariadb_dbplugin_obj
		EXPORT "${PROJECT_NAME}Targets"
	)
endif()
