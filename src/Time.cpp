/*
 * Time.cpp
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
#include <ecs/Time.hpp>
#include <chrono>
#include <ctime>
#include <mutex>

std::tm ecs::time::localtime(const std::time_t *time){
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	std::time_t t;

	if(time == nullptr) {
		t = std::time(nullptr);
	}else{
		t = *time;
	}
	std::tm tm = *std::localtime(&t);
	return tm;
}

std::tm ecs::time::gmtime(const std::time_t *time){
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	std::time_t t;
	
	if(time == nullptr) {
		t = std::time(nullptr);
	}else{
		t = *time;
	}
	std::tm tm = *std::gmtime(&t);
	return tm;
}
