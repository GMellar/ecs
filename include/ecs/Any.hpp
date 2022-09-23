/*
 * Any.hpp
 *
 *  Created on: 31.10.2015
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
#ifndef SRC_ECSTOOLS_ANY_HPP_
#define SRC_ECSTOOLS_ANY_HPP_

#include <typeinfo>
#include <string>
#include <utility>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <ecs/PointerDefinitions.hpp>
#include <ecs/memory.hpp>
#include <ecs/config.hpp>

namespace ecs {
namespace tools {
namespace any {

template<typename Tid>
class Any;

template<typename TType, typename TId, TId id>
struct AnyTypedef {
	using type = TType;
	using tid  = TId;
	/** Returns the id specified by id
	 */
	static tid getId(){
		return id;
	}

	operator TId() const {
		return id;
	}
};
	
class ECS_EXPORT AnyTypeBase {
public:
	POINTER_DEFINITIONS(AnyTypeBase);

	static int generateID() {
		static int id = 0;
		return ++id;
	}

	virtual ~AnyTypeBase() {

	}

	virtual AnyTypeBase::ptr_T clone() = 0;
	virtual const std::type_info &type() const noexcept = 0;
	virtual int typeInt() noexcept = 0;
};

template<typename TObject>
class AnyType: public AnyTypeBase {
public:
	POINTER_DEFINITIONS(AnyType);
	
	/* This is the actual object holding the 
	 * data.
	 */
	TObject object;

	AnyType(const TObject &obj) :
			object(obj) {

	}

	AnyType(TObject &&moveObj) : 
			object(std::move(moveObj)) {
		
	}

	virtual ~AnyType() {

	}

	/** Clones the type inside thsi object. 
	 */
	AnyTypeBase::ptr_T clone() {
		return new AnyType(object);
	}
	
	virtual const std::type_info &type() const noexcept {
		return typeid(object);
	}

	virtual int typeInt() noexcept {
		return getTypeInt();
	}

	static int getTypeInt() {
		static const int id = generateID();
		return id;
	}
};

template<typename Tid = std::string>
class Any {
public:
	POINTER_DEFINITIONS(Any);

	Any() : typeHolder(nullptr) {

	}

	Any(const Any &other) : typeHolder(nullptr) {
		clone(other);
	}

	Any(Any &&other) :
			typeHolder(nullptr) {
		clone(other);
	}

	/** Special cinstructor for constructing a type id which is
	 * mostly an id indicating that there is no data present.
	 * This is used by the database module to assign a NIL type id
	 * without an actual type holder.
	 *
	 * This means has_value can be used to check is the type is null.
	 */
	Any(std::nullptr_t n, const Tid &type) : typeHolder(nullptr), id(type) {

	}

	template<typename TObject>
	Any(TObject &value) :
			typeHolder(new AnyType<typename std::decay<TObject>::type>(value)) {

	}
	
	template<typename TObject>
	Any(TObject &&value,
			typename std::enable_if<!std::is_same<TObject, Any&>::value>::type* = nullptr) :
					typeHolder(new AnyType<typename std::remove_reference<TObject>::type>(std::move(value))) {

	}

	template<typename TObject>
	Any(TObject &value, const Tid &type) :
			typeHolder(new AnyType<typename std::decay<TObject>::type>(value)),id(type) {

	}
	
	template<typename TObject>
	Any(TObject &&value, const Tid &type,
			typename std::enable_if<!std::is_same<TObject, Any&>::value>::type* = nullptr) :
					typeHolder(new AnyType<typename std::decay<TObject>::type>(static_cast<TObject&&>(value))),
					id(type) {

	}

	virtual ~Any() {
		if (typeHolder != nullptr) delete typeHolder;
	}

	Any &operator=(const Any &other) {
		clone(other);
		return *this;
	}

	Any &operator=(Any &&other) {
		clone(other);
		return *this;
	}

	template<typename TObject>
	Any &operator=(const TObject &value) {
		Any(value).swap(*this);
		return *this;
	}

	template<typename TObject>
	Any &operator=(TObject &&value) {
		Any(value).swap(*this);
		return *this;
	}

	Any& swap(Any &other) {
		std::swap(other.typeHolder, typeHolder);
		std::swap(other.id, id);
		return *this;
	}

	const Tid& getTypeId() const {
		return id;
	}

	void setTypeId(const Tid& typeID) {
		this->id = typeID;
	}

	template<typename TObject>
	TObject *cast() const {
		typename AnyType<TObject>::ptr_T result;
		if (typeHolder != nullptr && AnyType<TObject>::getTypeInt() == typeHolder->typeInt()) {
			result = static_cast<typename AnyType<TObject>::ptr_T>(typeHolder);
			if (result != nullptr) {
				return &result->object;
			}
		}

		throw std::runtime_error("Invalid cast");
	}

	template<typename TObject>
	TObject &cast_reference() const {
		return *cast<TObject>();
	}
	
	bool has_value() const noexcept {
		return typeHolder != nullptr;
	}
	
	const std::type_info &type() const noexcept {
		return typeHolder->type();
	}

	explicit operator std::int64_t() const {
		return cast_reference<std::int64_t>();
	}

	virtual operator std::string() const {
		return cast_reference<std::string>();
	}
protected:
	AnyTypeBase::ptr_T typeHolder;

	Tid id;

	void clone(const Any &other){
		if (other.typeHolder != nullptr) {
			typeHolder = other.typeHolder->clone();
			id         = other.id;
		}
	}
};

/** Use this function to allocate a new Any object. This function returns a
 * pointert to the allocated object. You must give a class or struct as TType
 * which defines a static function to get the type. A struct could look like this:
 * \code
	struct String {
		using type = std::string;
		using tid = int;
		static tid getId(){
			return 0;
		}
	};
 * \endcode
 * In this example, type is the datatype the any class will hold. tid is the type
 * of the identifier and the static function getId() will get the identifier for this
 * type.
 */

template<typename TType, typename ...TData>
inline static typename Any<typename TType::tid>::ptr_T make(const TData &...data) {
	return new Any<typename TType::tid>(typename TType::type(data...), TType::getId());
}

template<typename TType, typename ...TData>
inline static typename Any<typename TType::tid>::sharedPtr_T make_shared(const TData &...data) {
	return std::make_shared<Any<typename TType::tid>>(typename TType::type(data...), TType::getId());
}

template<typename TType, typename ...TData>
inline static typename Any<typename TType::tid>::uniquePtr_T make_unique(const TData &...data) {
	return ecs::make_unique<Any<typename TType::tid>>(typename TType::type(data...), TType::getId());
}

template<typename TType>
typename TType::type &cast_reference(Any<typename TType::tid> &data){
	return data.template cast_reference<typename TType::type>();
}

template<class TType, class Tid = std::string>
TType &any_cast(const Any<Tid> &data) {
	return data.template cast_reference<TType>();
}

}
}
}

#endif /* SRC_ECSTOOLS_ANY_HPP_ */
