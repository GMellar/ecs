/*
 * Signals.hpp
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

#ifndef SRC_ECSTOOLS_SIGNALS_HPP_
#define SRC_ECSTOOLS_SIGNALS_HPP_

#include <ecs/config.hpp>
#include <string>
#include <list>
#include <map>
#include <set>
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>

namespace ecs {
namespace tools {

class SignalHandlerImpl;

/** Application specific signal handler to handle events like termination requests.
 * This is not intended as general event emitter. It is just
 */
class ECS_EXPORT SignalHandler {
public:
	typedef void (*signalFunction_T)(int);
	
	SignalHandler();
	
	SignalHandler(const SignalHandler &handler);

	SignalHandler(SignalHandler &&handler);

	SignalHandler& operator=(const SignalHandler &handler);

	SignalHandler& operator=(SignalHandler &&handler);

	virtual ~SignalHandler();
	
	/** Block the current thread until termination is requested.
	 * This one uses application wide signals. You don't have to care
	 * about threads.
	 */
	bool waitForTerminationRequest();

	/** Wait the given amount of time. When a termination has been requested, the
	 * function returns true, otherwhise false.
	 */
	bool waitForTerminationRequest(std::chrono::milliseconds millis);

	/** Set the stop request to true and inform all
	 * listeners that the application stop has been requested. This
	 * is independent from system signals so you can safely use this
	 * class to wait for a termination while another thread calls
	 * this function.
	 */
	void terminate();
private:
	std::shared_ptr<SignalHandlerImpl> impl;
};

}
}

#endif /* SRC_ECSTOOLS_OPTIONPARSER_HPP_ */
