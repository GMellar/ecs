/*
 * Error.hpp
 *
 *  Created on: Feb 13, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSTOOLS_ERROR_HPP_
#define SRC_ECSTOOLS_ERROR_HPP_

#include <ecs/config.hpp>

namespace ecs {

class ECS_EXPORT Error {
public:
	/** Constructs an error object which is not an error */
	Error();
	Error(unsigned int errorCode);
	virtual ~Error();
	virtual unsigned int getErrorCode() const;
	virtual void setErrorCode(unsigned int code);
	virtual operator bool() const;
protected:
	unsigned int errorCode;
};

}


#endif /* SRC_ECSTOOLS_ERROR_HPP_ */
