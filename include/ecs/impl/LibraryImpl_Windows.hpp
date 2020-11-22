/*
 * LibraryImpl_Windows.hpp
 *
 *  Created on: Mar 10, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDYNLIB_IMPL_LIBRARYIMPL_WINDOWS_HPP_
#define SRC_ECSDYNLIB_IMPL_LIBRARYIMPL_WINDOWS_HPP_

#include <windows.h>
#include <string>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/Library.hpp>
#include "LibraryImplBase.hpp"

namespace ecs {
namespace dynlib {

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

	bool isLoaded();
protected:
	/** Holds the handle for the shared library */
	HINSTANCE handle;
};

/** @} */

}
}

#endif /* SRC_ECSDYNLIB_IMPL_LIBRARYIMPL_WINDOWS_HPP_ */
