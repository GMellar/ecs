/*
 * Exception.hpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSTOOLS_EXCEPTION_HPP_
#define SRC_ECSTOOLS_EXCEPTION_HPP_

#include <string>
#include <exception>
#include <ecs/config.hpp>

namespace ecs {

/** This is the base class for all exceptions used in the
 * ecs project.
 */
class ECS_EXPORT Exception : public virtual std::exception {
public:
	virtual ~Exception();
	Exception();
	Exception(const std::string &message);

	virtual const char *what() const noexcept;
protected:
	std::string message;
};

}


#endif /* SRC_ECSTOOLS_EXCEPTION_HPP_ */
