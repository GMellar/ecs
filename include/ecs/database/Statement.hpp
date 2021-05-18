/*
 * Statement.hpp
 *
 *  Created on: 23.02.2016
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2017 Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SRC_ECSDB_DB3_STATEMENT_HPP_
#define SRC_ECSDB_DB3_STATEMENT_HPP_

#include <ecs/config.hpp>
#include <ecs/database/types.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/QueryResult.hpp>
#include <string>
#include <cstddef>
#include <memory>
#include <vector>
#include <streambuf>
#include <ecs/Library.hpp>



namespace ecs {
namespace db3 {
	
/** @addtogroup ecsdb
 * @{
 */

class StatementImpl;
class DbConnection;
class StatementInternals;

/** Class for statements. You never need to use this without shared pointers 
 * because statements are, depending on the plugin, holders of a connection 
 * and the statement implementation. Both, connection and statement implementation, 
 * are laoded plugins. 
 * 
 * Another reason for using shared pointers only is the facts, that statements 
 * can be stored inside classes to hold database queries for tasks. Statement creation 
 * is an expensive operation so you should keep them and reset them if needed. 
 * 
 * A statement is not threadsafe so you must be sure to access a statement from one thread only. 
 */
class ECS_EXPORT Statement : public std::enable_shared_from_this<Statement> {
	friend class DbConnection;
	friend class Result;
public:
	POINTER_DEFINITIONS(Statement);

	Statement(DbConnection *connection);

	Statement(const Statement &statement) = default;

	Statement(Statement &&statement) = default;

	Statement &operator=(const Statement &statement) = default;

	virtual ~Statement();
	
	bool bind(std::int64_t value);

	bool bind(const std::string &value);

	bool bind(double value);

	bool bind(float value);
	
	/** The streambuffer must be valid until the statement is done 
	 * so never put buffer pointers in here and return the statement. 
	 * The statement will be valid but the buffer is not. 
	 */
	bool bind(std::basic_streambuf<char> *streambuffer);
	
	/** This is the implementation for binding 
	* a null value.
	*/
	bool bind(std::nullptr_t);

	/** Execute the query. An execution will 
	 * give you a result which is valid until the statement 
	 * is valid. The result contains this statement as shared 
	 * pointer. If you access result values after execution of 
	 * reset the you may run into trouble. 
	 * 
	 * The result table is received in one peace so a bigh query
	 * may need a lot of memory.
	 *
	 * This function throws if there was a failure during the sql 
	 * statement execution. 
	 * 
	 * Remember: Result values are valid until the statement is 
	 * not reset. 
	 */
	Result execute();

	/** Get the last inserted row id. This may not be
	 * implemented in every database plugin and may throw.
	 * Thread safe for every connection.
	 */
	std::int64_t lastInsertId();

	/** Put back statement into initial state 
	 * This will call clearBinding so there is no need to do that.
	 */
	void reset();

	/** Remove all bindings */
	void clearBindings();
	
	/** Get the error message for a failed execution.
	 *
	 */
	std::string getErrorMessage() const;

protected:
	bool bind(ecs::db3::types::cell_T::ptr_T ptr);

	bool bind(ecs::db3::types::cell_T::uniquePtr_T &&ptr);
	
	/** bind NULL to statement. No longer 
	 * a public function.
	 */
	bool bind();
	
private:
	StatementInternals *impl;

	/** Fetch a single row from the result after calling
	 * execute.
	 */
	Row::uniquePtr_T fetch();
};

/** @} */

}
}



#endif /* SRC_ECSDB_DB3_STATEMENT_HPP_ */
