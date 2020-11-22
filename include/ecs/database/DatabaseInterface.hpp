/*
 * DatabaseInterface.hpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDB_DB3_DATABASEINTERFACE_HPP_
#define SRC_ECSDB_DB3_DATABASEINTERFACE_HPP_

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/database/Connection.hpp>
#include <memory>
#include <tuple>

namespace ecs {
namespace db3 {

/** Base class which provides a virtual function to get the database connection
 * to perform queries for different interfaces. This one is needed for constructing
 * interfaces.
 *
 * This class was introduced because there might be different method of getting
 * the database connection. Just implement this class to fit your needs.
 */
class ECS_EXPORT InterfaceConnectionBase {
public:
	POINTER_DEFINITIONS(InterfaceConnectionBase);

	virtual ~InterfaceConnectionBase();
	virtual DbConnection::sharedPtr_T getConnection() = 0;
	operator DbConnection::sharedPtr_T ();
protected:
	InterfaceConnectionBase();
};

/** The default interface connection implementation. It stores a database connection
 * inside. Keep in mind that the getConnection() function always returns the same
 * connection. You may clone that connection if you need different ones.
 *
 * In most cases this will suffice all needs.
 */
class ECS_EXPORT InterfaceConnection : public InterfaceConnectionBase {
public:
	POINTER_DEFINITIONS(InterfaceConnection);
	virtual ~InterfaceConnection();
	/** The connection is created by calling the connect function
	 * on the params object.
	 */
	InterfaceConnection(ConnectionParameters &params);
	InterfaceConnection(DbConnection::sharedPtr_T connection);
	virtual DbConnection::sharedPtr_T getConnection();

protected:
	DbConnection::sharedPtr_T connection;
};

class ECS_EXPORT InterfaceConnectionPerThread : public InterfaceConnectionBase {
public:
	virtual ~InterfaceConnectionPerThread();
	InterfaceConnectionPerThread(ConnectionParameters &params);
	virtual DbConnection::sharedPtr_T getConnection();
protected:
	ConnectionParameters params;
};

class ECS_EXPORT DatabaseInterfaceBase {
public:
	POINTER_DEFINITIONS(DatabaseInterfaceBase);
	virtual ~DatabaseInterfaceBase();

	DbConnection::sharedPtr_T getConnection();
	const DbConnection::sharedPtr_T getConnection() const;
	const ConnectionParameters &getParameters() const;
	/** Use this function when no connection should be
	 * used anymore and every call to getConnection should
	 * throw an exception. This will reset the connection pointer.
	 */
	void clearConnection();

protected:
	DatabaseInterfaceBase();
	DatabaseInterfaceBase(DbConnection::sharedPtr_T connection);
	DatabaseInterfaceBase(InterfaceConnectionBase::sharedPtr_T connection);

private:
	InterfaceConnectionBase::sharedPtr_T connection;
};

/** The database interface class is used to provide access to the
 * database without writing a lot of constructor code. There is an
 * interface builder class which holds the policy for connections.
 */
template<class TInterface>
class DatabaseInterface : public DatabaseInterfaceBase {
public:
	POINTER_DEFINITIONS(DatabaseInterface);

	virtual ~DatabaseInterface(){

	};

	template<class ...TConstructorArgs>
	static TInterface get(InterfaceConnectionBase &i, TConstructorArgs ...args) {
		/* This forces the implementation to implement at least one constructor
		 * with the connection as argument. This should be the normal situation.
		 * You can decide if later access to the database is required and use the second constructor
		 * which will initialize your connection as needed or just not call the second constructor which
		 * will omit later access to the database.
		 */
		TInterface result(i.getConnection(), args...);
		return result;
	}

protected:
	/** You are not forced to call the constructor of the base class with the connection
	 * but using this constructor will not initialize the connection variable and getConnection()
	 * will always throw.
	 *
	 * There might be cases where this is wanted e.g. pure reading classes without ever writing back data
	 * to the database.
	 */
	DatabaseInterface() {

	}

	DatabaseInterface(DbConnection::sharedPtr_T connection) : DatabaseInterfaceBase(connection) {

	}
};

}
}


#endif /* SRC_ECSDB_DB3_DATABASEINTERFACE_HPP_ */
