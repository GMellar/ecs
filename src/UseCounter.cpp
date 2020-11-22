
/*
 * UseCounter.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#include <ecs/UseCounter.hpp>
#include <atomic>
#include <cstdint>

namespace ecs {
namespace tools {

class UseCounterImpl {
public:
	UseCounterImpl() {
		counter = 0;
	}

	virtual ~UseCounterImpl() {

	}

	std::atomic<std::uint64_t> counter;
};

}
}

ecs::tools::UseCounter::UseCounter() {
	this->impl = new UseCounterImpl;
}

ecs::tools::UseCounter::~UseCounter() {
	delete impl;
	impl = nullptr;
}

void ecs::tools::UseCounter::increment() {

}

void ecs::tools::UseCounter::decrement() {

}
