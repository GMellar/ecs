/*
 * Library.hpp
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

#ifndef SRC_ECSDYNLIB_LIBRARY_HPP_
#define SRC_ECSDYNLIB_LIBRARY_HPP_

#include <ecs/config.hpp>
#include <functional>
#include <ecs/AbstractPlugin.hpp>
#include <ecs/LoadableClass.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <memory>
#include <string>
#include <map>
#include <string>
#include <exception>
#include <stdexcept>

/** @ingroup ecsdynlib
 *  @{
 */

#ifndef DYNLIB_NO_PLUGIN

#define DYNLIB_VERSION(major, minor, revision) \
	extern "C" const char * ecs_getVersion() { \
		return major "." minor "." revision; \
	}

#define DYNLIB_BEGIN_CLASS_DEFINITION(x)                          \
	extern "C" int ecs_registerClasses(                           \
			void *passBackToClassInserter,                        \
			void (*classInserter)(                                \
				void *,                                           \
				const char *,                                     \
				ecs::dynlib::LoadableClass*(*)(),                 \
				void (*)(ecs::dynlib::LoadableClass*)             \
			)                                                     \
		) {

#define DYNLIB_CLASS_DEFINITION(name, object) {\
			auto classConstructor  = []() -> ecs::dynlib::LoadableClass* {return new object();}; \
			auto classDeleter      = [](ecs::dynlib::LoadableClass *p) -> void {delete p;}; \
			classInserter( \
				passBackToClassInserter, \
				name, \
				classConstructor, \
				classDeleter \
			); \
		}

#define DYNLIB_END_CLASS_DEFINITION(x) return 0;}

#else 
	#define DYNLIB_VERSION(major, minor, revision)
	#define DYNLIB_BEGIN_CLASS_DEFINITION(x) 
	#define DYNLIB_CLASS_DEFINITION(name, object)
	#define DYNLIB_END_CLASS_DEFINITION(x)
#endif 

/** @} */

namespace ecs {
namespace dynlib {
	
/** @ingroup ecsdynlib
 *  @{
 */

class Library;

/** This is a wrapper class for loadable class in cases where the loadable
 * class does not inherit from LoadableClass.
 */
template<class TClass>
class Class {
	friend class Library;
public:
	POINTER_DEFINITIONS(Class);

	Class() {

	}

	Class(TClass *obj) : object(obj) {

	}
	
	Class(std::shared_ptr<TClass> obj) : object(obj) {
		
	}
	
	virtual ~Class(){
		/* Reset the object first before unloading the library.
		 * This is important because the library must still exist when
		 * the object is destructed. It is not allowed to unload a dynamic
		 * loaded library while there is still memory around.
		 */
		object.reset();
		dynLib.reset();
	}

	Class(const Class &c) = default;
	Class(Class &&c) = default;
	Class &operator=(const Class &c) = default;
	Class &operator=(Class &&c) = default;

	inline operator bool() const {
		if(object){
			return true;
		}
		return false;
	}

	/** Access to the stored object */
	inline TClass *operator->() const {
		return object.get();
	}

protected:
	/** Storage for the loaded library. This is
	 * the shared library created by the inline
	 * function.
	 */
	std::shared_ptr<Library> dynLib;
	/** Storage for the object. The object
	 * destruction happens inside the loaded module
	 * because custom deleters are defined.
	 */
	std::shared_ptr<TClass> object;
};

class LibraryImpl;

/** Represents a loadable library which can be loaded at runtime. It is
 * possible to execute function inside that library and load classed. Class
 * loading is supported by using macros to register them and a wrapper class
 * which holds the class beside the loaded library shared pointer.
 *
 * It is very important that no memory from synamic loaded libraries is still
 * there when unloading a library. When you write classes which are loaded at runtime
 * you must take care where the memory is deleted from these classes.
 *
 * The class itself has a custom deleter registered on the shared pointer creation but
 * memory from this class must be deleted manually inside that module with the same
 * delete function. This might be critical when using windows. When compiling your modules with the
 * same compiler version inside your project, this might not be a concern to you.
 */
class ECS_EXPORT Library {
public:
	POINTER_DEFINITIONS(Library);

	typedef int (*registerClassCallbackSignature)(void *, void (*)(
			void *,
			const char *,
			ecs::dynlib::LoadableClass*(*)(),
			void (*)(ecs::dynlib::LoadableClass*)));
	typedef ecs::dynlib::LoadableClass*(*classConstructorCallbackSignature)();
	typedef void (*classDeconstructorCallbackSignature)(ecs::dynlib::LoadableClass*);

	virtual ~Library();
protected:
	Library();

	Library(const Library &library) = delete;

	Library(Library &&library) = delete;

	Library &operator=(const Library &library) = delete;

	Library &operator=(Library &&library) = delete;

public:
	bool isLoaded() const;

	/** Returns true if the loaded library contains a C symbol with
	 * the given name.
	 */
	bool hasSymbol(const char *symbol);

	/** Get a C symbol with the given name. The result is
	 * a function pointer which is callable. You must know
	 * the return and function parameters.
	 */
	void *getSymbol(const char *symbol);
	
	/** Loads the library specified in filename.
	 * 
	 */
	static inline Library::sharedPtr_T load(const std::string &filename) {
		/* Load library */
		auto library = std::shared_ptr<Library>(new Library());

		/* Allocate implementation */
		library->impl = library->implementationBuilder(filename.c_str());

		if(library->isLoaded()){
			/* Receive loadable classes into map */
			auto classLoader = library->castSymbol<registerClassCallbackSignature>("ecs_registerClasses");

			if(classLoader){
				classLoader(library.get(), [] (void *thisClass, const char *name,
						ecs::dynlib::LoadableClass*(*classConstructor)(),
						void (*classDeleter)(ecs::dynlib::LoadableClass*)) -> void {
					static_cast<Library*>(thisClass)->insertClassCallback(name, classConstructor, classDeleter);
				});
			}

			return library;
		}

		throw std::runtime_error(std::string("Plugin ") + filename + std::string(" not loadable"));
	}

	template<typename TFunction>
	inline TFunction castSymbol(const std::string &symbol){
		return reinterpret_cast<TFunction>(getSymbol(symbol.c_str()));
	}

	/** Loads a class from a laodable module and stores the library shared pointer
	 * and the class shared pointer inside a wrapper class for using loadable classes.
	 * The wrapper is used to keep the shared library object inside and the loaded class.
	 * Copy this object around as you like.
	 *
	 * The loaded class is wrapped around a shared pointer with a custom deleter. The deleter
	 * function is defined with the DYNLIB_CLASS_DEFINITION macro inside your loadable module.
	 * This means the loaded class is destructed using the delete command from the loaded
	 * module and not from the loading executable. That results in encapsulating the deletion functions
	 * for using shared pointers beyond the shared library boundary.
	 *
	 * This is a static inlined function to be independend of any standard library implementation. You
	 * have to provide the shared pointer which has been created in yout application which is save because
	 * this function compiled in your application.
	 */
	template<class TClass>
	static inline Class<TClass> loadClass(Library::sharedPtr_T &lib, const std::string &name){
		Class<TClass> result;
		classConstructorCallbackSignature   constructor = nullptr;
		classDeconstructorCallbackSignature deleter     = nullptr;

		if(!lib) throw std::runtime_error("Library does not contain a valid object");

		/* find registered class name in the plugin */
		lib->getFactoryPointers(name.c_str(), &constructor, &deleter);

		if(constructor && deleter){
			auto loadedClass = std::shared_ptr<TClass>(static_cast<TClass*>(constructor()), deleter);

			if(!loadedClass) throw std::runtime_error("Class loading failed due to empty class pointer");

			/* Store this library inside the wrapper */
			result.dynLib = lib;
			/* Store the laoded class inside the wrapper */
			result.object = loadedClass;
		}else{
			throw std::runtime_error("Class '" + name + "' is not loadable");
		}

		return result;
	}

	/** This must not be called directly but it is called from inside the
	 * plugin.
	 */
	void insertClassCallback(const char *className,
			ecs::dynlib::LoadableClass *(*classConstructor)(),
			void (*classDeconstructor)(ecs::dynlib::LoadableClass *));

private:
	/** Holds the implementation details of the dynamic library
	 * loader.
	 */
	LibraryImpl *impl;
	
	/** Calls the implementations unload function. This is
	 * called inside the destructor of this class.
	 */
	bool unload();

	/** Build the implementation of the library. This functions
	 * is here because inline functions do not know anything about the implementation
	 * details.
	 */
	LibraryImpl *implementationBuilder(const char *filename);

	/** Get pointer to build the class.
	 *
	 */
	void getFactoryPointers(const char *className,
			classConstructorCallbackSignature *constructor,
			classDeconstructorCallbackSignature *destructor);
};

/** @} */

}
}



#endif /* SRC_ECSDYNLIB_LIBRARY_HPP_ */
