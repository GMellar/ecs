 
#ifndef ECS_CONFIG_HPP
#define ECS_CONFIG_HPP

#include <ecs/ecs_export.h>

#define ECS_DATABASE_PLUGINDIR ""
#define ECS_DATABASE_PLUGIN_EXTENSION ""

#cmakedefine ECS_HAVE_LIBUUID
#cmakedefine ECS_POSTGRESQL_DRIVER
#cmakedefine ECS_SQLITE3_DRIVER

#endif
