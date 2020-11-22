/*
 * Time.hpp
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
#ifndef SRC_ECSTOOLS_TIME_HPP_
#define SRC_ECSTOOLS_TIME_HPP_

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <ctime>

namespace ecs {
namespace time {

/** Thredsafe version of std::localtime */
ECS_EXPORT std::tm localtime(const std::time_t *time = nullptr);

/** Threadsafe version of std::gmtime */
ECS_EXPORT std::tm gmtime(const std::time_t *time = nullptr);

}
}


#endif /* SRC_ECSTOOLS_TIME_HPP_ */
