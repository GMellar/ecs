/*
 * TicToc.cpp
 *
 *  Created on: 10.04.2017
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

#include <ecs/TicToc.hpp>
#include <utility>

ecs::tools::TicToc::TicToc() {

}

ecs::tools::TicToc::~TicToc() {

}

void ecs::tools::TicToc::tic() {
	Tic t;
	t.startPoint = std::chrono::high_resolution_clock::now();
	tictocStack.push(std::move(t));
}

void ecs::tools::TicToc::tic ( const std::string &message ) {
	Tic t;
	t.startPoint = std::chrono::high_resolution_clock::now();
	t.message = message;
	tictocStack.push(std::move(t));
}


void ecs::tools::TicToc::toc ( std::ostream &stream ) {
	if(tictocStack.size() > 0){
		auto t = std::chrono::high_resolution_clock::now();
		auto startPoint = tictocStack.top();
		tictocStack.pop();
		auto difference = std::chrono::duration_cast<std::chrono::microseconds>(t - startPoint.startPoint).count();
		
		if(!startPoint.message.empty()){
			stream << startPoint.message << ": ";
		}
		stream << difference << " µs";
		stream << std::endl;
	}else{
		
	}
}

void ecs::tools::TicToc::toc() {
	toc(std::cout);
}

void ecs::tools::TicToc::clear() {
	std::stack<Tic> s;
	tictocStack.swap(s);
}
