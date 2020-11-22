/*
 * Library.cpp
 *
 *  Created on: 25.02.2016
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

#include <ecs/Library.hpp>
#ifdef ECS_PLATFORM_WINDOWS
	#include "impl/LibraryImpl_Windows.cpp"
#else
	#include "impl/LibraryImpl_dl.cpp"
#endif

#include <exception>
#include <string>

using namespace ecs::dynlib;

ecs::dynlib::Library::Library() : impl(nullptr) {

}

ecs::dynlib::Library::~Library() {
	delete impl;
}

bool ecs::dynlib::Library::hasSymbol(const char *symbol) {
	return impl->hasSymbol(symbol);
}

void* ecs::dynlib::Library::getSymbol(const char *symbol) {
	return impl->getSymbol(symbol);
}

bool ecs::dynlib::Library::unload() {
	return impl->unload();
}

void ecs::dynlib::Library::insertClassCallback(const char *className,
		ecs::dynlib::LoadableClass *(*classConstructor)(),
		void (*classDeconstructor)(ecs::dynlib::LoadableClass *)) {
	impl->pluginClasses.insert(std::make_pair(className, LibraryImplBase::ClassFunctions(classConstructor, classDeconstructor)));
}

LibraryImpl* ecs::dynlib::Library::implementationBuilder(const char *filename) {
	return new LibraryImpl(filename);
}

bool ecs::dynlib::Library::isLoaded() const {
	return impl->isLoaded();
}

void Library::getFactoryPointers(const char *className,
		classConstructorCallbackSignature *constructor,
		classDeconstructorCallbackSignature *destructor) {
	/* Search for the class in the loaded library */
	auto it = impl->pluginClasses.find(className);

	if(it != impl->pluginClasses.end()) {
		*constructor = it->second.constructClassCallback;
		*destructor  = it->second.destroyClassCallback;
	}else{
		*constructor = nullptr;
		*destructor  = nullptr;
	}
}
