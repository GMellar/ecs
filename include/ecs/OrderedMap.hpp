/*
 * OrderedMap.hpp
 *
 *  Created on: 11.03.2015
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
#ifndef SRC_ECSTOOLS_ORDEREDMAP_HPP_
#define SRC_ECSTOOLS_ORDEREDMAP_HPP_

#include <functional>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <list>
#include <deque>
#include <map>
#include <utility>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <ecs/PointerDefinitions.hpp>

namespace ecs {
namespace tools {

template<typename TKeyImpl, typename TKeyCompare>
class OrderedMapSet {
public:
	using set = boost::bimaps::set_of<TKeyImpl*, TKeyCompare>;
};

template<typename TKeyImpl, typename TKeyCompare>
class OrderedMapMultiset {
public:
	using set = boost::bimaps::multiset_of<TKeyImpl*, TKeyCompare>;
};

/** This template class implements an ordered map where you can
 * preserve the insertion order and find elements by a key.
 */
template<typename TKey, typename TData, template<typename TKeyImpl,typename TKeyCompare> class TSet = OrderedMapSet>
class OrderedMap {
public:
	POINTER_DEFINITIONS(OrderedMap);
	typedef std::pair<TKey, TData> value_type;
	typedef std::list<value_type> dataList_T;
	typedef typename dataList_T::iterator iterator;
	typedef typename dataList_T::const_iterator const_iterator;
	typedef typename dataList_T::size_type size_type;
	typedef typename std::pair<iterator,bool> insert_result;
	
	template<typename TKey_>
	struct keyCompare {
		bool operator()(TKey_ *key1, TKey_ *key2) const {
			return *key1 < *key2;
		}
	};
	template<typename TIterator>
	struct iteratorCompare {
		bool operator()(const TIterator &it1, const TIterator &it2) const {
			return *it1 < *it2;
		}
	};

	using keyMap_T = boost::bimap<typename TSet<TKey,keyCompare<TKey>>::set,
			boost::bimaps::multiset_of<iterator, iteratorCompare<iterator>>>;

	OrderedMap() {

	}

	virtual ~OrderedMap() {

	}
	
	insert_result push_back(TKey key, TData data) {
		iterator resultIterator;

		/* Push onto top of the list */
		this->data.push_back(value_type(key, data));
		resultIterator = std::prev(end());

		/* Put into map */
		auto result = keys.insert(
				typename keyMap_T::value_type(&resultIterator->first,
						resultIterator));

		if(result.second == false){
			this->data.pop_back();
			return std::make_pair(end(),false);
		}

		/* Return iterator */
		return std::make_pair(resultIterator,true);
	}
	
	insert_result push_front(TKey key, TData data) {
		iterator resultIterator;

		/* Push onto top of the list */
		this->data.push_front(value_type(key, data));
		resultIterator = begin();

		/* Put into map */
		auto result = keys.insert(
				typename keyMap_T::value_type(&resultIterator->first,
						resultIterator));

		if(result.second == false){
			this->data.pop_front();
			return std::make_pair(end(),false);
		}

		/* Return iterator */
		return std::make_pair(resultIterator,true);
	}

	void erase(iterator it) {
		/* Erase in the bimap */
		keys.right.erase(it);
		
		/* Erase in the list */
		data.erase(it);			
	}
	
	void erase(TKey key) {
		erase(find(key));
	}
	
	void clear() {
		data.clear();
		keys.clear();
	}

	iterator find(TKey key) {
		typename keyMap_T::left_iterator it = keys.left.find(&key);

		if (it != keys.left.end()) {
			return it->second;
		} else {
			return data.end();
		}
	}


	
	value_type &front() {
		return data.front();
	}

	value_type &back() {
		return data.back();
	}

	iterator begin() {
		return data.begin();
	}

	iterator end() {
		return data.end();
	}

	const_iterator begin() const {
		return data.begin();
	}

	const_iterator end() const {
		return data.end();
	}

	size_type size() const {
		return data.size();
	}
protected:
	dataList_T data;
	keyMap_T keys;

};

}
}

#endif /* SRC_ECSTOOLS_ORDEREDMAP_HPP_ */
