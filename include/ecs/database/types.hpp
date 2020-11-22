/*
 * types.hpp
 *
 *  Created on: 01.11.2015
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

#ifndef SRC_ECSDB_TYPES_HPP_
#define SRC_ECSDB_TYPES_HPP_

#include <ecs/Any.hpp>
#include <cstdint>
#include <streambuf>
#include <memory>

namespace ecs {
namespace db3 {
namespace types {

/** @addtogroup ecsdb 
 * @{
 */
	
enum class typeId {
	STRING = 0,
	NIL,
	INT64,
	UINT64,
	DOUBLE,
	BLOB
};

using cell_T = ecs::tools::any::Any<typeId>;

struct Null : ecs::tools::any::AnyTypedef<void*, typeId, typeId::NIL> {
	
};

struct String : ecs::tools::any::AnyTypedef<std::string, typeId, typeId::STRING> {
	
};

struct Int64 : ecs::tools::any::AnyTypedef<int64_t, typeId, typeId::INT64> {
	
};

struct Uint64 : ecs::tools::any::AnyTypedef<uint64_t, typeId, typeId::UINT64> {
	
};

struct Double : ecs::tools::any::AnyTypedef<double, typeId, typeId::DOUBLE> {
	
};

struct Blob : ecs::tools::any::AnyTypedef<std::shared_ptr<std::basic_streambuf<char>>, typeId, typeId::BLOB> {
	
};

/** @} */

}
}
}


#endif /* SRC_ECSDB_TYPES_HPP_ */
