/*
 * Exception.hpp
 *
 *  Created on: Feb 6, 2018
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 */

#ifndef SRC_ECSDB_DB3_EXCEPTION_HPP_
#define SRC_ECSDB_DB3_EXCEPTION_HPP_

#include <ecs/Exception.hpp>
#include <ecs/config.hpp>

namespace ecs {
namespace db3 {
namespace exceptions {

/** Base class for all database related exception.
 *
 */
class ECS_EXPORT Exception : public ecs::Exception {
public:
	Exception();
	Exception(const std::string &message);
	virtual ~Exception();
};

}
}
}


#endif /* SRC_ECSDB_DB3_EXCEPTION_HPP_ */
