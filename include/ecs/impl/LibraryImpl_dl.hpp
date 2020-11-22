/*
 * LibraryImpl_dl.hpp
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

#ifndef _LIBRARYIMPL_HPP_
#define _LIBRARYIMPL_HPP_

#include <string>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/Library.hpp>
#include "LibraryImplBase.hpp"

#include <dlfcn.h>

namespace ecs {
namespace dynlib {

/** @ingroup ecsdynlib
 * @{
 */
	
class LibraryImpl : public LibraryImplBase {
	friend class Library;
public:
	POINTER_DEFINITIONS(LibraryImpl);

	LibraryImpl(const std::string &path);

	LibraryImpl(const LibraryImpl &library) = default;

	LibraryImpl(LibraryImpl &&library) = default;

	LibraryImpl &operator=(const LibraryImpl &library) = default;

	LibraryImpl &operator=(LibraryImpl &&library) = default;

	virtual ~LibraryImpl();

	bool unload();

	bool load (const std::string &path);

	bool hasSymbol(const std::string &symbol);

	void *getSymbol(const std::string &symbol);

	bool isLoaded() const;
protected:
	/** Holds the handle for the shared library */
	void *handle;
};

/** @} */

}
}


#endif
