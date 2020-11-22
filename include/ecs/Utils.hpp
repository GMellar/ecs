/*
 * Utils.hpp
 *
 *  Created on: 02.03.2013
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

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <ecs/config.hpp>
#include <string>
#include <cstdint>

namespace ecs {
namespace tools {

/** @addtogroup ecstools
 * @{
 */

/** Generate Random UUID
 *
 */
ECS_EXPORT std::string randomUUID();

/** Generate Time Based UUID
 * @deprecated Do not use time based uuids
 */
ECS_EXPORT std::string uuid();

/** Generate Timestamp in ISO 8601 format.
 *
 */
ECS_EXPORT std::string timestamp(int lifetime = 0);

ECS_EXPORT uint64_t timestampInt();

ECS_EXPORT std::string getpassword(const char *prompt, bool show_asterisk = true);

ECS_EXPORT std::string getInput(const char *prompt);

ECS_EXPORT int stringToUint64_t(std::string number, uint64_t &result);

/** @} */

}  // namespace tools
}  // namespace ecs

#endif /* UTILS_HPP_ */
