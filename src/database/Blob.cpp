/*
 * Blob.cpp
 *
 *  Created on: 11.12.2014
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
 
#include <ecs/database/Blob.hpp>
#include <iostream>

ecs::db3::BlobBuffer::BlobBuffer() {
	setg(nullptr, nullptr, nullptr);
	setp(nullptr, nullptr);
}

ecs::db3::BlobBuffer::~BlobBuffer() {

}

ecs::db3::BlobBuffer::int_type ecs::db3::BlobBuffer::underflow() {
	return std::basic_streambuf<char>::underflow();
}

ecs::db3::BlobBuffer::int_type ecs::db3::BlobBuffer::overflow ( int ch ) {
	return std::basic_streambuf<char>::overflow();
}
