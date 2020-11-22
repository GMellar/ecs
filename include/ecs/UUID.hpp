/*
 * UUID.hpp
 *
 *  Created on: 01.12.2015
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

#ifndef SRC_ECSTOOLS_UUID_HPP_
#define SRC_ECSTOOLS_UUID_HPP_

#include <ecs/config.hpp>
#include <string>
#include <cstdint>

namespace ecs {
namespace tools {

class UUID;

/** Random UUID generator version 4*/
class ECS_EXPORT UUIDGenerator {
public:
	virtual ~UUIDGenerator();
	UUIDGenerator();

	virtual void generate(UUID &target) const;
};

/** Generates UUID version 2 with the security extension if
 * needed.
 */
class ECS_EXPORT UUIDGenerator2 : public UUIDGenerator {
public:
	virtual ~UUIDGenerator2();
	UUIDGenerator2(bool useSecure = true);

	virtual void generate(UUID &target) const;

protected:
	bool useSecure;
};

class ECS_EXPORT UUID {
	friend class UUIDGenerator;
	friend class UUIDGenerator2;
public:
	virtual ~UUID();
	UUID();
	UUID(const UUIDGenerator &generator);

	std::string toString() const;
protected:
	std::string data;
};

}
}


#endif /* SRC_ECSTOOLS_UUID_HPP_ */
