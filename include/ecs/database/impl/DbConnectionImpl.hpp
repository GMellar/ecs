/*
 * DbConnectionImpl.hpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDB_DB3_IMPL_DBCONNECTIONIMPL_HPP_
#define SRC_ECSDB_DB3_IMPL_DBCONNECTIONIMPL_HPP_

#include <ecs/Library.hpp>
#include <ecs/database/Connection.hpp>
#include <ecs/database/ConnectionParameters.hpp>
#include <ecs/database/impl/ConnectionImpl.hpp>
#include <memory>

namespace ecs {
namespace db3 {

class DbConnectionImpl {
public:
	DbConnectionImpl();
	virtual ~DbConnectionImpl();

	/** Make a copy of the connection implementation which means
	 * the internals state is still active and the module is kept
	 * laoded.
	 */
	DbConnectionImpl *clone();

	/** Holds the connection implementation (Loaded plugin).
	 * The plugin is valid as long as the connection exists.
	 */
	ecs::dynlib::Class<ConnectionImpl> module;

	/** Connection parameters valid for this connection. These
	 * are the parameters the loader received to establish
	 * a connection.
	 */
	ecs::db3::ConnectionParameters parameters;
};

}
}

#endif /* SRC_ECSDB_DB3_IMPL_DBCONNECTIONIMPL_HPP_ */
