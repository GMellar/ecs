#ifndef ECS_DATABASE_HEADER_
#define ECS_DATABASE_HEADER_

#include <ecs/config.hpp>
#include <ecs/database/Exception.hpp>
#include <ecs/database/types.hpp>
#include <ecs/database/Blob.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/ConnectionParameters.hpp>
#include <ecs/database/Connector.hpp>
#include <ecs/database/Migrator.hpp>
#include <ecs/database/Plugin.hpp>
#include <ecs/database/QueryResult.hpp>
#include <ecs/database/Row.hpp>
#include <ecs/database/Statement.hpp>
#include <ecs/database/Table.hpp>
#include <ecs/database/DatabaseInterface.hpp>

#define SQL_QUERY(...) #__VA_ARGS__

#endif
 
