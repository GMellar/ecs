/*
 * Error.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/Error.hpp>
#include <ecs/config.hpp>

ecs::Error::Error() : errorCode(0) {
}

ecs::Error::Error(unsigned int errorCode) : errorCode(errorCode) {
}

ecs::Error::~Error() {
}

unsigned int ecs::Error::getErrorCode() const {
	return errorCode;
}

void ecs::Error::setErrorCode(unsigned int code) {
	errorCode = code;
}

ecs::Error::operator bool() const {
	return errorCode == 0;
}
