/*
 * PointerDefinitions.hpp
 *
 *  Created on: 08.10.2015
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
#ifndef SRC_ECSTOOLS_POINTERDEFINITIONS_HPP_
#define SRC_ECSTOOLS_POINTERDEFINITIONS_HPP_

#include <memory>

/** Define a pointer of the class and a shared pointer of the class.
 * This macro can only be used in a class
 */
#define POINTER_DEFINITIONS(x) using ptr_T = x*; \
	using sharedPtr_T = std::shared_ptr<x>; \
	using uniquePtr_T = std::unique_ptr<x>; \
	using weakPtr_T   = std::weak_ptr<x>

#endif /* SRC_ECSTOOLS_POINTERDEFINITIONS_HPP_ */
