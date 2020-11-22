/*
 * Observable.hpp
 *
 *  Created on: 05.12.2015
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

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <ecs/Function.hpp>
#include <list>
#include <memory>
#include <utility>
#include <atomic>

namespace ecs {
namespace tools { 

/** Observable template class. Can hold subscriptions 
 * for notifying class members and normal functions on 
 * value change. You can use simple types or inherit from 
 * this observable class to make your class observable. In 
 * This case you must notice the observable class about the pointer 
 * by using the constructor Observable(TObservable *) and set this 
 * as parameter. Then the observable handler class knows about 
 * the object but never manages memory. 
 */
template<class TObservable>
class Observable {
public:
	Observable(){
		observableData = std::make_shared<TObservable>();
	}
	
	/** Use this when using another class as observable 
	 * by default. When you inherit a class from observable 
	 * you need to use this observable constructor 
	 */
	Observable(TObservable *ptr){
		observableData.reset(ptr, [](TObservable *p){});
	}
	
	template<class... TParams>
	Observable(TParams... params) {
		observableData = std::make_shared<TObservable>(params...);
	}
	
	virtual ~Observable(){
		
	}
	
	TObservable &operator()() {
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator()(TData &data) {
		*observableData = data;
		notify();
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator=(TData &data) {
		*observableData = data;
		notify();
		return *observableData;
	}

	template<class TData>
	TObservable &operator-=(TData &data) {
		*observableData -= data;
		notify();
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator+=(TData &data) {
		*observableData += data;
		notify();
		return *observableData;
	}

	template<class TData>
	TObservable &operator/=(TData &data) {
		*observableData /= data;
		notify();
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator%=(TData &data) {
		*observableData %= data;
		notify();
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator&=(TData &data) {
		*observableData &= data;
		notify();
		return *observableData;
	}
	
	template<class TData>
	TObservable &operator|=(TData &data) {
		*observableData |= data;
		notify();
		return *observableData;
	}
	
	/** Adds a subscription. There are some things to consider 
	 * at the moment when using subscriptions. The first thing is that you 
	 * must not change the observable from within your subscription handler. 
	 * This is very importand because it would end in an infinite loop. This 
	 * will change in later versions. The second point is that you should 
	 * keep the handlers as short as possible. 
	 */
	template<class ...TParam>
	Observable &subscribe(TParam... fnParams){
		subscriptions.push_back(ecs::tools::Function<void(TObservable&)>(fnParams...));
		return *this;
	}
	
protected:
	/** Call this function when the observable data changes.
	 * Notify will call all subscribers.
	 */
	void notify() {
		for(auto it : subscriptions){
			it(*observableData);
		}
	}
	
private:
	std::list<ecs::tools::Function<void(TObservable&)>> subscriptions;
	std::shared_ptr<TObservable>                        observableData;
};

}
}

#endif // OBSERVABLE_H
