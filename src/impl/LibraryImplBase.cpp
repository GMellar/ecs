/*
 * LibraryImplBase.cpp
 *
 *  Created on: Oct 19, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/impl/LibraryImplBase.hpp>

ecs::dynlib::LibraryImplBase::LibraryImplBase() {

}

ecs::dynlib::LibraryImplBase::~LibraryImplBase() {

}

ecs::dynlib::LibraryImplBase::ClassFunctions::ClassFunctions(
		ecs::dynlib::Library::classConstructorCallbackSignature constructor,
		ecs::dynlib::Library::classDeconstructorCallbackSignature deleter) {
	this->constructClassCallback = constructor;
	this->destroyClassCallback   = deleter;
}

ecs::dynlib::LibraryImplBase::ClassFunctions::~ClassFunctions() {
}
