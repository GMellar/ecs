/*
 * Signleton.hpp
 *
 *  Created on: 10.04.2017
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

#ifndef SINGLETON_H
#define SINGLETON_H

#if __cplusplus <= 199711L
  #error This library needs at least a C++11 compliant compiler
#endif

namespace ecs {
namespace tools {

template<class TClass>
class SingleInstance {
public:
	SingleInstance(SingleInstance *instance) : instance(instance) {
		
	}
	
	virtual ~SingleInstance() {
		
	}
	
	TClass *getInstance() {
		static_cast<TClass*>(instance);	
	}
private:
	SingleInstance *instance;
};

/** Singleton class. This class does not prevent multiple instances
 * but provides a getInstance function. This is for classes which
 * should have a single global instance but allow more than one
 * lcoal instance.
 *
 * Though singleton sound really great for unexperienced developers, they
 * are not. You wonder why these class is present anyways? Because experienced
 * developers may know when to use them. Everything is possible without singletons
 * so this is just for quick and dirty solutions.
 */
template<class TClass>
class Singleton
{
public:
	Singleton(){
		
	};
	
	virtual ~Singleton(){
		
	};
	
	static TClass &getInstance(){
		static TClass instance;
		return instance;
	}
};

/** Same as singleton but the global object is local for every thread.
 *
 */
template<class TClass>
class SingletonThreadLocal
{
public:
	SingletonThreadLocal(){

	};

	virtual ~SingletonThreadLocal(){

	};

	static TClass &getInstance(){
		static thread_local TClass instance;
		return instance;
	}
};

}
}

#endif // SINGLETON_H
