/*
 * Timestamp.hpp
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

#ifndef SRC_ECSTOOLS_TIMESTAMP_HPP_
#define SRC_ECSTOOLS_TIMESTAMP_HPP_

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <memory>
#include <chrono>
#include <cstdint>
#include <string>

namespace ecs {
namespace time {

class TimestampImpl;

/** @todo complete */
class Timestamp {
	friend class TimestampImpl;
public:
	POINTER_DEFINITIONS(Timestamp);

	Timestamp();

	virtual ~Timestamp();

protected:
	std::shared_ptr<TimestampImpl> impl;
};

namespace ISO8601 {
	ECS_EXPORT std::string now(std::chrono::seconds offset = std::chrono::seconds(0), bool localtime = false);
}

}
}


#endif /* SRC_ECSTOOLS_TIMESTAMP_HPP_ */
