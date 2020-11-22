/*
 * LibraryImpl_Windows.cpp
 *
 *  Created on: Mar 10, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/impl/LibraryImpl_Windows.hpp>

ecs::dynlib::LibraryImpl::LibraryImpl(const std::string &path) : handle(NULL) {
	load(path);
}

ecs::dynlib::LibraryImpl::~LibraryImpl() {
	unload();
}

bool ecs::dynlib::LibraryImpl::hasSymbol(const std::string& symbol) {
	if(handle != NULL && GetProcAddress(handle, symbol.c_str()) != NULL){
		return true;
	}
	return false;
}

void* ecs::dynlib::LibraryImpl::getSymbol(const std::string& symbol) {
	if(hasSymbol(symbol)){
		return (void*)GetProcAddress(handle, symbol.c_str());
	}
	return nullptr;
}

bool ecs::dynlib::LibraryImpl::load(const std::string& path) {
	handle = LoadLibrary(path.c_str());
	if(handle != NULL){
		return true;
	}
	return false;
}

bool ecs::dynlib::LibraryImpl::isLoaded() {
	if(handle == NULL){
		return false;
	}
	return true;
}

bool ecs::dynlib::LibraryImpl::unload() {
	if(handle != NULL){
		FreeLibrary(handle);
		handle = NULL;
		return true;
	}
	return false;
}
