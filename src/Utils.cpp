/*
 * Utils.cpp
 *
 *  Created on: 02.03.2013
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
#include <ecs/Utils.hpp>
#include <ecs/Time.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <time.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>

#ifdef WINDOWS
	#include <windows.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

using namespace std;

static boost::uuids::random_generator randomUUIDGenerator;
static std::mutex randomUUIDGeneratorMutex;

std::string ecs::tools::uuid() {
	return randomUUID();
}

std::string ecs::tools::randomUUID() {
	std::lock_guard<std::mutex> lock(randomUUIDGeneratorMutex);
	auto uuid = randomUUIDGenerator();
	return boost::uuids::to_string(uuid);;
}

std::string ecs::tools::timestamp(int lifetime) {
	auto now       = std::chrono::system_clock::now() + std::chrono::seconds(lifetime);
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	auto gmt       = ecs::time::gmtime(&in_time_t);
	char formattedTime[100];
	
	std::strftime((char*)&formattedTime, sizeof(formattedTime)/sizeof(char), "%FT%T%z", &gmt);
	return std::string((char*)&formattedTime);
}

static void SetStdinEcho(bool enable = true)
{
#ifdef WINDOWS
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

std::string ecs::tools::getpassword(const char *prompt, bool show_asterisk) {
	vector<string> result;
	int character;
	cout << prompt;
	SetStdinEcho(false);
	while(1){
		character = getchar();
		if(character == '\n')break;
		result.push_back(string((char*)(&character)));
	}
	SetStdinEcho(true);
	cout << endl;
	return boost::algorithm::join(result,"");
}

int ecs::tools::stringToUint64_t(std::string number, uint64_t& result) {
	try{
		result = boost::lexical_cast<uint64_t>(number);
		return 0;
	}catch(std::exception &e){
		return -1;
	}
}

std::string ecs::tools::getInput(const char* prompt) {
	std::string result;
	std::cout << prompt;
	std::cin >> result;
	return result;
}
