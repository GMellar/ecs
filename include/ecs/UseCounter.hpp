/*
 * UseCounter.hpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSTOOLS_USECOUNTER_HPP_
#define SRC_ECSTOOLS_USECOUNTER_HPP_

#include <ecs/config.hpp>

namespace ecs {
namespace tools {

class UseCounterImpl;

class ECS_EXPORT UseCounter {
public:
	UseCounter();
	virtual ~UseCounter();

	/** Increment use count */
	void increment();
	/** Decrement use count */
	void decrement();


private:
	UseCounterImpl *impl;
};

}
}


#endif /* SRC_ECSTOOLS_USECOUNTER_HPP_ */
