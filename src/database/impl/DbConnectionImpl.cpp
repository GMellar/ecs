/*
 * DbConnectionImpl.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/database/impl/DbConnectionImpl.hpp>

ecs::db3::DbConnectionImpl::DbConnectionImpl() {

}

ecs::db3::DbConnectionImpl::~DbConnectionImpl() {

}

ecs::db3::DbConnectionImpl* ecs::db3::DbConnectionImpl::clone() {
	std::unique_ptr<DbConnectionImpl> result(new DbConnectionImpl());
	*result = *this;
	return result.release();
}
