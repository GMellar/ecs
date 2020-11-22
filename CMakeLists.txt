cmake_minimum_required(VERSION 3.10)
project(ecs LANGUAGES C CXX VERSION 0.4.1)

enable_testing()

include(GenerateExportHeader)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

find_library(DL_LIBRARY NAMES dl)
find_package(Threads)
find_package(Boost REQUIRED COMPONENTS system filesystem) 

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/catch2")

file(GLOB ecs_sources_base 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/impl/LibraryImplBase.cpp")

file(GLOB ecs_database_base 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/sqlite3/sqlite3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/sqlite3/sqlite3.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/sqlite3/UUID.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/sqlite3/UTC.cpp")
add_library(ecs_database_obj OBJECT ${ecs_database_base})
target_include_directories(ecs_database_obj
	PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_BINARY_DIR}/include"
		${Boost_INCLUDE_DIRS}
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_compile_definitions(ecs_database_obj PRIVATE SQLITE_CORE )

add_library(${PROJECT_NAME} STATIC 
		${ecs_sources_base}
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_BINARY_DIR}/include"
		${Boost_INCLUDE_DIRS}
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC 
		${Boost_LIBRARIES}
	PRIVATE 
		Threads::Threads
		ecs_database_obj
	INTERFACE 
		$<BUILD_INTERFACE:Threads::Threads>
		$<BUILD_INTERFACE:${Boost_LIBRARIES}>
)

if(WIN32) 
	target_compile_definitions(${PROJECT_NAME} 
		PUBLIC ECS_PLATFORM_WINDOWS
	)
endif()

if(DL_LIBRARY) 
	target_link_libraries(${PROJECT_NAME} 
		PUBLIC 
			${DL_LIBRARY})
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/config.hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/config.hpp")
generate_export_header(${PROJECT_NAME} 
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/ecs_export.h")
	
add_executable(ecs_test "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp")
target_link_libraries(ecs_test ${PROJECT_NAME} Catch2)
