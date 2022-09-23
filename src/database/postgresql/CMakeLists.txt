# Find postgresql package for linking
find_package(PostgreSQL)

option(ECS_POSTGRESQL_DRIVER "Enable Postgresql database driver" FALSE)

if(NOT ${ECS_POSTGRESQL_DRIVER})
	return()
endif()

if(NOT ${PostgreSQL_FOUND})
	message(WARNING "Did not find postgresql but building was requested")
	return()
endif()

if(${PostgreSQL_FOUND} AND PostgreSQL_LIBRARIES AND PostgreSQL_INCLUDE_DIRS)

else()
	message(WARNING "Did not find postgresql but building was requested")
	return()
endif()

file(GLOB postgresql_sources
	"${CMAKE_CURRENT_SOURCE_DIR}/postgresql.cpp")

add_library(postgresql_dbplugin_obj OBJECT ${postgresql_sources})
target_compile_definitions(postgresql_dbplugin_obj 
	PRIVATE DYNLIB_NO_PLUGIN)
target_include_directories(postgresql_dbplugin_obj 
	PRIVATE 
		$<TARGET_PROPERTY:ecs,INTERFACE_INCLUDE_DIRECTORIES>
		${PostgreSQL_TYPE_INCLUDE_DIR}
		${PostgreSQL_INCLUDE_DIR}
	INTERFACE
		$<BUILD_INTERFACE:${PostgreSQL_INCLUDE_DIR}>
		$<BUILD_INTERFACE:${PostgreSQL_TYPE_INCLUDE_DIR}>
)
		
target_link_libraries(postgresql_dbplugin_obj 
	PRIVATE
		PostgreSQL::PostgreSQL
	INTERFACE
		PostgreSQL::PostgreSQL
)

install(TARGETS postgresql_dbplugin_obj
	EXPORT "${PROJECT_NAME}Targets"
)

add_library(postgresql_dbplugin MODULE ${postgresql_sources})
target_link_libraries(postgresql_dbplugin 
	PRIVATE 
		ecs
		PostgreSQL::PostgreSQL
)
target_include_directories(postgresql_dbplugin 
	PRIVATE
		$<TARGET_PROPERTY:postgresql_dbplugin_obj,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(postgresql_dbplugin PROPERTIES OUTPUT_NAME postgresql)
set_target_properties(postgresql_dbplugin PROPERTIES PREFIX "")
install(TARGETS postgresql_dbplugin
	LIBRARY DESTINATION lib/ecs/dbplugin
)
