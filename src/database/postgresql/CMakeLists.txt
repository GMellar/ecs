# Find postgresql package for linking
find_package(PostgreSQL REQUIRED)

if(NOT ${PostgreSQL_FOUND})
	return()
endif()

find_path(PostgreSQL_libpq_INCLUDE_DIR
	NAMES libpq-fe.h
	PATHS ${PostgreSQL_INCLUDE_DIRS}
	PATH_SUFFIXES
		postgresql
		include/postgresql
		pgsql/server
		postgresql/server
		include/server
)

add_library(postgresql_dbplugin_obj INTERFACE)
target_link_libraries(postgresql_dbplugin_obj 
	INTERFACE 
		$<BUILD_INTERFACE:${PostgreSQL_LIBRARIES}>)
		
target_include_directories(postgresql_dbplugin_obj 
	INTERFACE 
		$<BUILD_INTERFACE:${PostgreSQL_INCLUDE_DIRS}>
		$<BUILD_INTERFACE:${PostgreSQL_libpq_INCLUDE_DIR}>)

add_library(postgresql_dbplugin MODULE "${CMAKE_CURRENT_SOURCE_DIR}/postgresql.cpp")
target_link_libraries(postgresql_dbplugin postgresql_dbplugin_obj ecs)
target_include_directories(postgresql_dbplugin 
	PRIVATE 
		$<TARGET_PROPERTY:ecs,INTERFACE_INCLUDE_DIRECTORIES>)
		