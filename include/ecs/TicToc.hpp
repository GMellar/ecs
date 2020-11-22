/*
 * TicToc.hpp
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

#ifndef TICTOC_H
#define TICTOC_H

#include <chrono>
#include <iostream>
#include <string>
#include <stack>
#include <ecs/config.hpp>

namespace ecs {
namespace tools {

class ECS_EXPORT TicToc {
public:
	struct Tic {
		std::chrono::high_resolution_clock::time_point startPoint;
		std::string                                    message;
	};
	
	TicToc();
	
	virtual ~TicToc();
	
	void tic();
	
	void tic(const std::string &message);
	
	void toc();

	void toc(std::ostream &stream);
	
	void clear();
protected:
	std::stack<Tic> tictocStack;	
};


}
}
#endif // TICTOC_H
