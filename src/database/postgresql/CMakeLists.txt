# Find postgresql package for linking
find_package(PostgreSQL)

if(NOT ${PostgreSQL_FOUND})
	return()
endif()

add_library(postgresql_dbplugin MODULE "${CMAKE_CURRENT_SOURCE_DIR}/postgresql.cpp")
target_link_libraries(postgresql_dbplugin ${PostgreSQL_LIBRARIES} ecsdb ecstools)
target_include_directories(postgresql_dbplugin PRIVATE ${PostgreSQL_INCLUDE_DIRS})

##########################################################
# Set some properties
##########################################################
set_target_properties(postgresql_dbplugin PROPERTIES PREFIX "")
set_target_properties(postgresql_dbplugin PROPERTIES SUFFIX ${ECS_DATABASE_PLUGIN_EXTENSION})
set_target_properties(postgresql_dbplugin PROPERTIES OUTPUT_NAME "postgresql")
set_target_properties(postgresql_dbplugin PROPERTIES NO_SONAME TRUE)
set_target_properties(postgresql_dbplugin PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/plugins/database"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins/database"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/database"
)

##########################################################
# Installation
##########################################################
install(TARGETS postgresql_dbplugin 
	DESTINATION "${ECS_DATABASE_PLUGINDIR}" 
	COMPONENT db_bin)
