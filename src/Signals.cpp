/*
 * Signals.cpp
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

#include <ecs/Signals.hpp>
#include <csignal>
#include <functional>
#include <set>

#if defined(__LINUX__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class ecs::tools::SignalHandlerImpl : public std::enable_shared_from_this<ecs::tools::SignalHandlerImpl> {
public:
	typedef void (*signalFunction_T)(int);

	SignalHandlerImpl(){
		signal(SIGINT,  &SignalHandlerImpl::signalHandler);
		signal(SIGABRT, &SignalHandlerImpl::signalHandler);
		signal(SIGTERM, &SignalHandlerImpl::signalHandler);
	}

	virtual ~SignalHandlerImpl(){
		std::lock_guard<std::mutex> lock(instancesMutex);

		for(auto instance = instances.begin(); instance != instances.end();) {
			if(!instance->lock()) {
				instance = instances.erase(instance);
			}else{
				instance++;
			}
		}
	}

	void init() {
		std::lock_guard<std::mutex> lock(instancesMutex);
		instances.push_back(shared_from_this());
	}

	bool waitForTerminationRequest(){
		if(terminationRequested == true){
			return true;
		}

		while(terminationRequested == false) {
			std::unique_lock<std::mutex> lock(terminationRequestedMutex);
			terminationRequestedCond.wait_for(lock, std::chrono::milliseconds(500));
		}

		return true;
	}

	bool waitForTerminationRequest(std::chrono::milliseconds millis){
		if(terminationRequested == true){
			return true;
		}

		while(terminationRequested == false) {
			std::unique_lock<std::mutex> lock(terminationRequestedMutex);
			terminationRequestedCond.wait_for(lock, millis);
		}

		return true;
	}

	void terminate() {
		terminationRequested = true;
		terminationRequestedCond.notify_all();
	}

	/** Variable is true when there was a termination request */
	static std::atomic<bool> terminationRequested;
	/** Mutex for the termination request */
	static std::mutex terminationRequestedMutex;
	/** Contition which receives a signal on termination request */
	static std::condition_variable terminationRequestedCond;

	/** Mutex on instances to keep the instances container clean.
	 *
	 */
	static std::mutex instancesMutex;

	/** This map contains every instance of this class. When you derive
	 * from this class, the constructor will put a pointer of this object
	 * into the container. When an object is deleted, then
	 * the handler will be deleted in the map too, because the destructor
	 * of this object will do it for you.
	 */
	static std::list<std::weak_ptr<SignalHandlerImpl>> instances;

	/** This is the static signal handler for catching incoming
	 * signals.
	 */
	static void signalHandler(int num){
		std::lock_guard<std::mutex> lock(instancesMutex);

		switch(num){
			case SIGABRT:
				terminationRequested = true;
				break;
			case SIGTERM:
				terminationRequested = true;
				break;
			case SIGINT:
				terminationRequested = true;
				break;
		}

		/* Call handlers */
		/** @todo implement handler caller */

		/* Raise the signal if termination requested was called */
		if(terminationRequested == true){
			terminationRequestedCond.notify_all();
		}
	}
};

std::mutex ecs::tools::SignalHandlerImpl::instancesMutex;
std::list<std::weak_ptr<ecs::tools::SignalHandlerImpl>> ecs::tools::SignalHandlerImpl::instances;
std::atomic<bool> ecs::tools::SignalHandlerImpl::terminationRequested(false);
std::condition_variable ecs::tools::SignalHandlerImpl::terminationRequestedCond;
std::mutex ecs::tools::SignalHandlerImpl::terminationRequestedMutex;

ecs::tools::SignalHandler::SignalHandler(){
	impl = std::make_shared<SignalHandlerImpl>();
	impl->init();
}

ecs::tools::SignalHandler::SignalHandler(const SignalHandler &handler){
	impl = std::make_shared<SignalHandlerImpl>();
	impl->init();
}

ecs::tools::SignalHandler::SignalHandler(SignalHandler &&handler){
	impl = std::make_shared<SignalHandlerImpl>();
	impl->init();
}

ecs::tools::SignalHandler& ecs::tools::SignalHandler::operator=(const SignalHandler &handler){
	impl = std::make_shared<SignalHandlerImpl>();
	impl->init();
	return *this;
}

ecs::tools::SignalHandler& ecs::tools::SignalHandler::operator=(SignalHandler &&handler){
	impl = std::make_shared<SignalHandlerImpl>();
	impl->init();
	return *this;
}

ecs::tools::SignalHandler::~SignalHandler(){

}

bool ecs::tools::SignalHandler::waitForTerminationRequest(){
	return impl->waitForTerminationRequest();
}

bool ecs::tools::SignalHandler::waitForTerminationRequest(std::chrono::milliseconds millis) {
	return impl->waitForTerminationRequest(millis);
}

void ecs::tools::SignalHandler::terminate(){
	impl->terminate();
}
