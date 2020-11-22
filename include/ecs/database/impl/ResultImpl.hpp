/*
 * ResultImpl.hpp
 *
 *  Created on: Mar 29, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDB_DB3_IMPL_RESULTIMPL_HPP_
#define SRC_ECSDB_DB3_IMPL_RESULTIMPL_HPP_

#include <memory>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/Row.hpp>
#include <ecs/database/Table.hpp>
#include <ecs/database/impl/StatementImpl.hpp>

namespace ecs {
namespace db3 {

/** Base class for the result implementation.
 *
 */
class ECS_EXPORT ResultImpl {
	friend class Statement;
	friend class Result;
public:
	POINTER_DEFINITIONS(ResultImpl);

	/** Statement implementation. This is needed because the table
	 * holds values from inside the statement. Statement holds it's
	 * connection so the connection will be up until there are
	 * no more statements and results.
	 */
	StatementInternals *stmt;

	/** Result table. The decision to use
	 * a uniquePtr_T was made because it is faster
	 * and result copying isn't really needed. If you need to copy
	 * the result you have to use move semantics.
	 *
	 * It is considered bad to carry results accross functions. Every
	 * function should use an atomic operation on database results. If you
	 * need to call subroutines you can pass pointers or references of the
	 * result table.
	 */
	Table::uniquePtr_T         resultTable;

	virtual ~ResultImpl();

protected:
	ResultImpl(StatementInternals *stmt);
};

}
}

#endif /* SRC_ECSDB_DB3_IMPL_RESULTIMPL_HPP_ */
