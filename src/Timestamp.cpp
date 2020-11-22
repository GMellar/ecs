/*
 * Timestamp.cpp
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

#include <ecs/Timestamp.hpp>
#include <ecs/Time.hpp>
#include <chrono>

std::string ecs::time::ISO8601::now(std::chrono::seconds offset, bool localtime) {
	using namespace std::chrono;
	char timeString[64];
	auto now = system_clock::now() + offset;
	auto tt  = system_clock::to_time_t(now);
	std::tm tm;
	if (localtime == true){
		tm = ecs::time::localtime(&tt);
		std::strftime(timeString, sizeof(timeString), "%FT%T%z", &tm);
	}else{
		tm = ecs::time::gmtime(&tt);
		std::strftime(timeString, sizeof(timeString), "%FT%TZ", &tm);
	}
	return std::string(timeString);	
}
