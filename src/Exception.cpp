/*
 * Exception.cpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/Exception.hpp>

ecs::Exception::~Exception() {

}

ecs::Exception::Exception() {

}

ecs::Exception::Exception(const std::string& message) : message(message) {

}

const char* ecs::Exception::what() const noexcept {
	return message.c_str();
}
