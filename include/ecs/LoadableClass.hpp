/*
 * LoadableClass.hpp
 *
 *  Created on: 25.02.2016
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

#ifndef SRC_ECSDYNLIB_LOADABLECLASS_HPP_
#define SRC_ECSDYNLIB_LOADABLECLASS_HPP_

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <memory>
#include <string>


	
namespace ecs {
namespace dynlib {

/** \addtogroup ecsdynlib
 *  @{
 */
class Library;

/** This class represents a loadable class which can be
 * a parent class and contains the dynamic library shared
 * pointer to keep the library loaded while this class exists.
 */
class ECS_EXPORT LoadableClass {
	friend class Library;
public:
	POINTER_DEFINITIONS(LoadableClass);

	LoadableClass();

	virtual ~LoadableClass();

private:

};

/** @} */
}
}



#endif /* SRC_ECSDYNLIB_LOADABLECLASS_HPP_ */
