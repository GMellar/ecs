/*
 * Trigger.hpp
 *
 *  Created on: 15.12.2014
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

#ifndef TRIGGER_HPP_
#define TRIGGER_HPP_

#include <unordered_map>
#include <functional>
#include <memory>
#include <exception>

namespace ecs {
namespace tools {

/** @addtogroup ecstools
 * @{
 */

/** Class for a single trigger function. This is the class
 * used by Trigger2 to make use of triggers.
 */
template<typename TReturn, typename ... TArgs>
class TriggerFunction {
public:
	typedef std::function<TReturn(TArgs...)> function_T;

	TriggerFunction(){

	}

	TriggerFunction(function_T f) :
			function(f) {
	}
	
	TriggerFunction(TriggerFunction &&f) = default;
	
	template<typename TObj>
	TriggerFunction(function_T f, TObj obj) {
		function = [f,obj](TArgs... args) {
			return (obj->*f)(args...);
		};
	}

	virtual ~TriggerFunction() {

	}

	TReturn raiseTrigger(TArgs ... args) {
		if(function){
			return function(args...);
		}else{
			throw std::bad_function_call();
		}
	}

	void setFunction(function_T f) {
		function = f;
	}

	function_T &getFunction(){
		return function;
	}

	template<typename TFunction, typename TObj>
	void setFunction(TFunction f, TObj obj) {
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
	
	TriggerFunction &operator=(function_T f){
		function = f;
		return *this;
	}
	
	TReturn operator()(TArgs ... args){
		if(function){
			return function(args...);
		}else{
			throw std::bad_function_call();
		}
	}
protected:
	function_T function;
};

/** This class has the same function as the trigger class but registered
 * functions can take more than one argument by using variadic templates.
 */
template<typename TReturn, typename TKey, typename... TArgs>
class Trigger2 {
public:
	typedef std::unordered_map<TKey, TriggerFunction<TReturn, TArgs...> > triggerMap_T;

	Trigger2(){
		triggers.reset(new triggerMap_T);
	}

	virtual ~Trigger2(){

	};

	template<typename TFunction>
	bool addTriggerAcceptor(TKey key, TFunction f){
		typename triggerMap_T::iterator it;

		it = triggers->find(key);
		if(it != triggers->end()){
			return false;
		}else{
			triggers->insert(std::make_pair(key,TriggerFunction<TReturn, TArgs...>(f)));
		}
		return true;
	}

	template<typename TFunction, typename TObject>
	bool addTriggerAcceptor(TKey key, TFunction f, TObject obj){
		typename triggerMap_T::iterator it;

		it = triggers->find(key);
		if(it != triggers->end()){
			return false;
		}else{
			triggers->insert(std::make_pair(key,
				TriggerFunction<TReturn, TArgs...>([f,obj](TArgs... args) {
					return (obj->*f)(args...);
				})
			));
		}
		return true;
	};

	TReturn raiseTrigger(TKey key, TArgs... args){
		typename triggerMap_T::iterator it = triggers->find(key);

		if(it != triggers->end()){
			return it->second.raiseTrigger(args...);
		}else{
			/* Throw exception */
			throw std::bad_function_call();
		}
	}

	bool hasTrigger(TKey key){
		typename triggerMap_T::iterator it;

		it = triggers->find(key);
		if(it != triggers->end()){
			return true;
		}else{
			return false;
		}
	}

private:
	std::shared_ptr<triggerMap_T> triggers;
};

/** @} */

}
}


#endif /* TRIGGER_HPP_ */
