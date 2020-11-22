/*
 * StateMachine.hpp
 *
 *  Created on: 10.04.2015
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

#ifndef _SRC_ECSTOOLS_STATEMACHINE_HPP_
#define _SRC_ECSTOOLS_STATEMACHINE_HPP_

#include <functional>
#include <vector>
#include <list>
#include <memory>
#include <ecs/memory.hpp>
#include <ecs/PointerDefinitions.hpp>

namespace ecs {
namespace statemachine {

/** Default basse state for all states. This 
* base class is empty and intended as default parameter 
* for the state container. 
*/
struct StateBase {
	StateBase() {
	
	}
	
	virtual ~StateBase() {
	
	}
};

/** Container class for all states. This class 
* must provide a template function addState which returns 
* a specific state type. The container implementation is up to 
* you so implement whatever you need as a container and additional data.
* Only addState is a fixed requirement
*/
template<class TBaseState = StateBase>
struct Container {
	std::vector<std::shared_ptr<TBaseState>> states;
	
	template<class TMachine, class TState, class... TParam>
	TState *addState(TParam... params){
		auto state = std::make_shared<TState>(params...);
		states.push_back(state);
		return state.get();
	}
};

template<class TBaseState = StateBase>
struct ContainerUnique {
	std::vector<std::unique_ptr<TBaseState>> states;
	
	template<class TMachine, class TState, class... TParam>
	TState *addState(TParam... params){
		auto state = ecs::make_unique<TState>(params...);
		auto statePtr = state.get();
		states.push_back(std::move(state));
		return statePtr;
	}
};

template<class TBaseState = StateBase>
struct ListContainer {
	std::list<std::shared_ptr<TBaseState>> states;
	
	template<class TMachine, class TState, class... TParam>
	TState *addState(TParam... params){
		auto state = std::make_shared<TState>(params...);
		states.push_back(state);
		return state.get();
	}
};

template<class A, class B, class C, class ...D>
struct Machine;

template<class TMachine, template <class TStateBase> class TContainer, class TStateBase, class TReturn, class... TData>
struct Machine<TMachine, TContainer<TStateBase>, TReturn, TData...> : public TContainer<TStateBase> {	
	struct State : public TStateBase {
		/** Machine pointer which makes it possible to access 
		 * the machine from within a state. We need this to trigger 
		 * transitions from within a state.
		 */
		TMachine *machine;
		
		State() {
		
		}
		
		State(TMachine *m) : machine(m) {
		
		}
				
		virtual ~State(){
		
		}
		
		/** Is called when a new data event comes in. 
		 */
		virtual TReturn receive(TData... params) {
		
		}
		
		/** This is called when the state is activates which means 
		 * a transition happens. 
		 */
		virtual void onActivate(State *previousState) {
			
		}
		
		/** This function is called when this state is no longer 
		 * the active state.
		 */
		virtual void onDeactivate(State *newState) {
			
		}
		
		/** Set this state as active state in the machine. 
		 */
		void setActive() {
			machine->setActiveState(this);
		}
	};
	
	struct LambdaState : public State {
		std::function<TReturn(TData...)> lambda;
		
		LambdaState() {
		
		}
		
		LambdaState(TMachine *m) : State(m) {
		
		}
				
		LambdaState(std::function<TReturn(TData...)> fn) : lambda(fn) {
		
		}
		
		virtual ~LambdaState(){
		
		}
		
		LambdaState &onReceive(std::function<TReturn(TData...)> fn){
			lambda = fn;
		}
		
		virtual TReturn receive(TData... params) {
			return lambda(params...);
		}		
	};
	
	/** Current state which is selected */
	State *activeState;
	
	Machine() : activeState(nullptr) {
		
	}
	
	virtual ~Machine() {
	
	}
	
	/** Call this function to insert new data to the 
	 * machine.
	 */
	virtual TReturn receive(TData... params) {
		return activeState->receive(params...);
	}
	
	inline void setActiveState(State *s){
		/* Store old state */
		auto previousState = activeState;
		/* Set new state */
		activeState = s;
		/* Call deactivate handler */
		previousState->onDeactivate(activeState);
		/* Call activate handler */
		activeState->onActivate(previousState);
	}
	
	/** Adds a new state. Depending on your container implementation 
	 * the returned pointer must be freed by the user but every default container 
	 * has its own memory management so you must not free the pointer. If you implement 
	 * another container you must. It is highly recommended to use containers with automatic 
	 * memory management like vector to shared_ptr.
	 */
	template<class TState, class... TParam>
	inline TState *addState(TParam... params) {
		/* Add state using the container */
		TState *state = TContainer<TStateBase>::template addState<
			Machine<
				TMachine, TContainer<TStateBase>, TReturn, TData...>, 
				TState
			>(params...);
		
		/* Set the machine of this state */
		state->machine = static_cast<TMachine*>(this);
		
		/* Return reference to new created state */
		return state;
	}
};

}
}

#endif
