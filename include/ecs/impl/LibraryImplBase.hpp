/*
 * LibraryImplBase.hpp
 *
 *  Created on: Oct 19, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDYNLIB_IMPL_LIBRARYIMPLBASE_HPP_
#define SRC_ECSDYNLIB_IMPL_LIBRARYIMPLBASE_HPP_

#include <map>
#include <ecs/Library.hpp>

namespace ecs {
namespace dynlib {

class LibraryImplBase {
public:
	LibraryImplBase();
	virtual ~LibraryImplBase();

	struct ClassFunctions {
		ClassFunctions(ecs::dynlib::Library::classConstructorCallbackSignature constructor,
				ecs::dynlib::Library::classDeconstructorCallbackSignature deleter);
		virtual ~ClassFunctions();

		ecs::dynlib::Library::classConstructorCallbackSignature   constructClassCallback;
		ecs::dynlib::Library::classDeconstructorCallbackSignature destroyClassCallback;
	};

	std::map<std::string, ClassFunctions> pluginClasses;
};

}
}


#endif /* SRC_ECSDYNLIB_IMPL_LIBRARYIMPLBASE_HPP_ */
