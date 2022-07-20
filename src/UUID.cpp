/*
 * UUID.cpp
 *
 *  Created on: 01.12.2015
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *      GPG-ID: DBFC815C
 */


#include <ecs/UUID.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#ifdef ECS_HAVE_LIBUUID
#include <uuid/uuid.h>
#endif
#include <mutex>

static boost::uuids::random_generator randomUUIDGenerator;
static std::mutex                     randomUUIDGeneratorMutex;

ecs::tools::UUIDGenerator::~UUIDGenerator() {
}

ecs::tools::UUIDGenerator::UUIDGenerator() {
}

void ecs::tools::UUIDGenerator::generate(UUID& target) const {
	std::lock_guard<std::mutex> lock(randomUUIDGeneratorMutex);
	auto uuid = randomUUIDGenerator();
	target.data = boost::uuids::to_string(uuid);
}

ecs::tools::UUID::UUID() {
	
}

ecs::tools::UUID::~UUID() {
	
}

ecs::tools::UUID::UUID(const UUIDGenerator& generator) {
	generator.generate(*this);
}

std::string ecs::tools::UUID::toString() const {
	return data;
}

#ifdef ECS_HAVE_LIBUUID
ecs::tools::UUIDGenerator2::~UUIDGenerator2() {

}

ecs::tools::UUIDGenerator2::UUIDGenerator2(bool useSecure) : useSecure(useSecure) {

}

void ecs::tools::UUIDGenerator2::generate(UUID& target) const {
	uuid_t out;
	if(useSecure) {
		uuid_generate_time_safe(out);
	}else {
		uuid_generate_time(out);
	}

	char uuidStr[37];
	uuid_unparse_lower(out, uuidStr);
	target.data.assign((const char *)&uuidStr[0], 36);
}
#endif
