# Find postgresql package for linking
find_package(PostgreSQL)

if(NOT ${PostgreSQL_FOUND})
	return()
endif()

option(ECS_POSTGRESQL_DRIVER "Enable Postgresql database driver" TRUE)

if(NOT ${ECS_POSTGRESQL_DRIVER})
	set(postgresql_dbplugin_obj)
	return()
endif()

file(GLOB postgresql_sources
	"${CMAKE_CURRENT_SOURCE_DIR}/postgresql.cpp")


find_library(PostgreSQL NAMES pq)

find_path(PostgreSQL_postgres_INCLUDE_DIR
	NAMES postgres.h
	PATH_SUFFIXES
		postgresql/11/server
)

find_path(PostgreSQL_libpq_INCLUDE_DIR
	NAMES libpq-fe.h
	PATH_SUFFIXES
		postgresql
		include/postgresql
		pgsql/server
		postgresql/server
		include/server
)

add_library(postgresql_dbplugin_obj OBJECT ${postgresql_sources})
target_compile_definitions(postgresql_dbplugin_obj 
	PRIVATE DYNLIB_NO_PLUGIN)
target_include_directories(postgresql_dbplugin_obj 
	PRIVATE 
		$<TARGET_PROPERTY:ecs,INTERFACE_INCLUDE_DIRECTORIES>
		${PostgreSQL_postgres_INCLUDE_DIR}
		${PostgreSQL_libpq_INCLUDE_DIR}
	PUBLIC 
		$<BUILD_INTERFACE:${PostgreSQL_postgres_INCLUDE_DIR}>
		$<BUILD_INTERFACE:${PostgreSQL_libpq_INCLUDE_DIR}>
	INTERFACE 
		$<BUILD_INTERFACE:${PostgreSQL_postgres_INCLUDE_DIR}>
		$<BUILD_INTERFACE:${PostgreSQL_libpq_INCLUDE_DIR}>			
)
		
target_link_libraries(postgresql_dbplugin_obj 
	INTERFACE
		${PostgreSQL}
)

install(TARGETS postgresql_dbplugin_obj
	EXPORT "${PROJECT_NAME}Targets"
)

add_library(postgresql_dbplugin MODULE ${postgresql_sources})
target_link_libraries(postgresql_dbplugin 
	PRIVATE 
		ecs
		$<TARGET_PROPERTY:postgresql_dbplugin_obj,INTERFACE_LINK_LIBRARIES>
)
target_include_directories(postgresql_dbplugin 
	PRIVATE
		$<TARGET_PROPERTY:postgresql_dbplugin_obj,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(postgresql_dbplugin PROPERTIES OUTPUT_NAME postgresql)
set_target_properties(postgresql_dbplugin PROPERTIES PREFIX "")
install(TARGETS postgresql_dbplugin
	LIBRARY DESTINATION lib/ecs/dbplugin
)