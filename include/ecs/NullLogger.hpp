/*
 * NullLogger.hpp
 *
 *  Created on: Feb 20, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSTOOLS_NULLLOGGER_HPP_
#define SRC_ECSTOOLS_NULLLOGGER_HPP_


namespace ecs {
namespace tools {

class NullLogger {
public:
	template<class T>
	NullLogger(T) {}
	NullLogger() {}

	template<class T>
	NullLogger &operator<<(const T &){
		return *this;
	}
};

}
}


#endif /* SRC_ECSTOOLS_NULLLOGGER_HPP_ */
