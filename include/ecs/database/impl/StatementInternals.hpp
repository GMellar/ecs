/*
 * StatementInternals.hpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDB_DB3_IMPL_STATEMENTINTERNALS_HPP_
#define SRC_ECSDB_DB3_IMPL_STATEMENTINTERNALS_HPP_

#include <ecs/database/impl/StatementImpl.hpp>
#include <ecs/database/impl/DbConnectionImpl.hpp>
#include <ecs/database/Connection.hpp>
#include <memory>

namespace ecs {
namespace db3 {

class StatementInternals {
public:
	StatementInternals(DbConnection     *connection);
	StatementInternals(DbConnectionImpl *connection);
	virtual ~StatementInternals();

	/** Clones the statement internals to keep
	 * the connection alive.
	 */
	StatementInternals *clone();

	/** Holds the implementation (This is a plugin) loaded
	 * by the database connection. Tho keep the plugin loaded
	 * we need to store the connection implementation as well.
	 */
	std::shared_ptr<StatementImpl> stmt;

	/** Database connection implementation This keeps
	 * the loaded connection module.
	 * We need a shared pointer here
	 * because the database connection may
	 * be used in multiple statements at the same time.
	 * When there are no more statements and no more
	 * shared connections, then the database handle is closed.
	 */
	DbConnectionImpl *connection;

	/** Contains all bindings for the statement.
	 * We keep them here because all bindings
	 * need to be valid while the statement exists.
	 * The bound value may be stored inside the statement
	 * implementation if required but is resides here
	 * as well.
	 */
	std::vector<ecs::db3::types::cell_T::uniquePtr_T> bindings;
};

}
}



#endif /* SRC_ECSDB_DB3_IMPL_STATEMENTINTERNALS_HPP_ */
