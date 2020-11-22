/*
 * Exception.cpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/database/Exception.hpp>

ecs::db3::exceptions::Exception::Exception() {

}

ecs::db3::exceptions::Exception::Exception(const std::string& message) : ecs::Exception(message) {

}

ecs::db3::exceptions::Exception::~Exception() {

}
