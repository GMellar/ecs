/*
 * Function.hpp
 *
 *  Created on: 13.11.2015
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

#ifndef SRC_ECSTOOLS_FUNCTION_HPP_
#define SRC_ECSTOOLS_FUNCTION_HPP_


#include <utility>
#include <functional>
#include <ecs/config.hpp>

namespace ecs {
namespace tools {
	
template<class T>
class Function;

template<class TReturn, class ...TArgs>
class Function<TReturn(TArgs...)> {
public:
	typedef std::function<TReturn(TArgs...)> function_T;

	Function(){

	}
	
	template<typename TFn>
	Function(TFn f) : function(f) {
		
	}
	
	//Function(Function &&f) = default;
	
	template<typename TFn, typename TObj>
	Function(TFn f, TObj obj) {
		function = [f,obj](TArgs... args) {
			return (obj->*f)(args...);
		};
	}

	virtual ~Function() {

	}

	TReturn operator()(TArgs ... args) {
		if(function){
			return function(args...);
		}else{
			throw std::bad_function_call();
		}
	}
	
	Function &operator=(function_T f){
		function = f;
		return *this;
	}
	
	void set(function_T f) {
		function = f;
	}

	function_T &get(){
		return function;
	}

	template<typename TFunction, typename TObj>
	void set(TFunction f, TObj obj) {
		function = [f,obj](TArgs... args) {
			return (obj->*f)(args...);
		};
	}
	
	template<typename TFunction, typename TObj>
	void bind(TFunction f, TObj obj) {
		function = [f,obj](TArgs... args) {
			return (obj->*f)(args...);
		};
	}
	
protected:
	function_T function;
};

}
}


#endif /* SRC_ECSTOOLS_FUNCTION_HPP_ */
