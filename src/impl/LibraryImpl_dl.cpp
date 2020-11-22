/*
 * LibraryImpl_dl.cpp
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

#include <ecs/impl/LibraryImpl_dl.hpp>

ecs::dynlib::LibraryImpl::LibraryImpl(const std::string &path) : handle(NULL) {
	load(path);
}

ecs::dynlib::LibraryImpl::~LibraryImpl() {
	unload();
}

bool ecs::dynlib::LibraryImpl::hasSymbol(const std::string& symbol) {
	if(handle != NULL && dlsym(handle,symbol.c_str()) != NULL)return true;
	return false;
}

void* ecs::dynlib::LibraryImpl::getSymbol(const std::string& symbol) {
	if(hasSymbol(symbol)){
		return dlsym(handle,symbol.c_str());
	}
	return nullptr;
}

bool ecs::dynlib::LibraryImpl::load(const std::string& path) {
	handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
	if(handle != NULL){
		return true;
	}
	return false;
}

bool ecs::dynlib::LibraryImpl::isLoaded() const {
	if(handle == NULL){
		return false;
	}
	return true;
}

bool ecs::dynlib::LibraryImpl::unload() {
	if(handle != NULL){
		dlclose(handle);
		handle = NULL;
		return true;
	}
	return false;
}
