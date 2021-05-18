/*
 * ResultImpl.cpp
 *
 *  Created on: Mar 29, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/database/impl/ResultImpl.hpp>
#include <ecs/database/impl/StatementInternals.hpp>

ecs::db3::ResultImpl::~ResultImpl() {

}

ecs::db3::ResultImpl::ResultImpl(std::shared_ptr<Statement> stmt) : stmt(stmt) {

}
