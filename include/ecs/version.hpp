/*
 * version.hpp
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

#ifndef ECSTOOLS_VERSION_H
#define ECSTOOLS_VERSION_H

#include <string>
#include <ecs/config.hpp>

namespace ecs {
namespace tools {

int ECSTOOLS_EXPORT getVersionMajor();	
int ECSTOOLS_EXPORT getVersionMinor();
int ECSTOOLS_EXPORT getVersionPatch();
const char *getVersionInfo();
std::string ECSTOOLS_EXPORT getVersionString();
	
}
}

#endif // VERSION_H
