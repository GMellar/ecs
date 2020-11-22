/*
 * NameValueContainer.hpp
 *
 *  Created on: 28.02.2015
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
#ifndef SRC_ECSTOOLS_NAMEVALUECONTAINER_HPP_
#define SRC_ECSTOOLS_NAMEVALUECONTAINER_HPP_

#include <string>
#include <map>
#include <exception>
#include <functional>
#include <ecs/config.hpp>

namespace ecs {
namespace tools {

template<typename TValueType>
class NameValuePair {
public:
	NameValuePair(){

	}

	NameValuePair(std::string name, TValueType value) : name(name), value(value) {

	}

protected:
	std::string name;
	TValueType value;
};

template<typename TValueType>
class NameValueContainer {
public:
	NameValueContainer(){

	}

	NameValueContainer(const NameValueContainer &other) = default;
	
	NameValueContainer(NameValueContainer &&container) = default;

	virtual ~NameValueContainer(){

	}

	NameValueContainer &operator = (const NameValueContainer &other) = default;
	
	void iterate(std::function<void(const std::string &, const TValueType &)> fn) const {
		for(auto it : nameValueData){
			fn(it.first, it.second);
		}
	}

	int insert(const std::string &name, const TValueType &value){
		nameValueData[name] = value;
		return 0;
	}

	TValueType &get(const std::string &name){
		typename std::map<std::string,TValueType>::iterator it = nameValueData.find(name);
		if(it == nameValueData.end()){
			throw std::runtime_error("Value not found");
		}else{
			return it->second;
		}
	}

	TValueType &get(const std::string &name, const TValueType &defaultValue){
		typename std::map<std::string,TValueType>::iterator it = nameValueData.find(name);
		if(it == nameValueData.end()){
			return defaultValue;
		}else{
			return it->second;
		}
	}

	bool find(const std::string &name, TValueType &result) const {
		typename std::map<std::string,TValueType>::const_iterator it = nameValueData.find(name);
		if(it == nameValueData.end()){
			return false;
		}else{
			result = it->second;
			return true;
		}
	}

	virtual void clear(){
		nameValueData.clear();
	}

	void swap(NameValueContainer<TValueType> &container){
		nameValueData.swap(container.nameValueData);
	}
protected:
	std::map<std::string,TValueType> nameValueData;
};

}
}

#endif /* SRC_ECSTOOLS_NAMEVALUECONTAINER_HPP_ */
