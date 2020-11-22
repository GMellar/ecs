/*
 * Blob.hpp
 *
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

#ifndef BLOB_H
#define BLOB_H

#include <ecs/config.hpp>
#include <ecs/PointerDefinitions.hpp>
#include <streambuf>
#include <vector>
#include <list>

namespace ecs {
namespace db3 {

/** @addtogroup ecsdb 
 * @{
 */
	
class BlobImpl;
	
/** Base class for all binary Blobs 
 * 
 */
class ECS_EXPORT BlobBuffer : public std::basic_streambuf<char> {
public:
	POINTER_DEFINITIONS(BlobBuffer);
	
	BlobBuffer();
	
	virtual ~BlobBuffer();
	
	virtual int_type underflow();
	
	virtual int_type overflow(int_type ch);
};

/** @} */

}
}

#endif // BLOB_H
