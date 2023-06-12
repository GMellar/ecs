/*
 * BlobSource.hpp
 *
 *  Created on: 09.06.2023
 *      Author: Geoffrey Mellar <Geoffrey.Mellar@rub.de>
 */

#ifndef ECS_INCLUDE_ECS_DATABASE_BLOBSOURCE_HPP_
#define ECS_INCLUDE_ECS_DATABASE_BLOBSOURCE_HPP_

#include <ios>
#include <memory>
#include <vector>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/stream_buffer.hpp>

namespace ecs {
namespace db3 {

class BlobSource {
public:
	struct category
	: public boost::iostreams::device_tag,
	  public boost::iostreams::input_seekable
	{ };

	using char_type = char;

	std::shared_ptr<std::vector<char_type>> blobData;
	std::size_t                             pos;

	BlobSource(void *blob, std::size_t dataBytes) : blobData(std::make_shared<std::vector<char_type>>(dataBytes)), pos(0) {
		std::memcpy(reinterpret_cast<void*>(blobData->data()), blob, blobData->size());
	}

	BlobSource(std::size_t dataBytes) :
		blobData(std::make_shared<std::vector<char_type>>(dataBytes)), pos(0) {
	}

	BlobSource(const std::shared_ptr<std::vector<char_type>> &blobData) :
		blobData(blobData), pos(0) {
	}

	char_type *data() {
		return blobData->data();
	}

	std::size_t size() {
		return blobData->size();
	}

	std::streamsize read(char *s, std::streamsize n) {
		// Read up to n characters from the input
		// sequence into the buffer s, returning
		// the number of characters read, or -1
		// to indicate end-of-sequence.

		std::streamsize amt = static_cast<std::streamsize>(blobData->size() - pos);
		std::streamsize result = std::min(n, amt);
		if (result != 0) {
			std::copy(blobData->data() + pos, blobData->data() + pos + result,
					s);
			pos += result;
			return result;
		} else {
			return -1; // EOF
		}
	}

	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off,
			std::ios_base::seekdir way) {
		boost::iostreams::stream_offset next;
		if (way == std::ios_base::beg) {
			next = off;
		} else if (way == std::ios_base::cur) {
			next = pos + off;
		} else if (way == std::ios_base::end) {
			next = blobData->size() + off;
		} else {
			throw std::ios_base::failure("bad seek direction");
		}

		if (next < 0
				|| next
						> static_cast<boost::iostreams::stream_offset>(blobData->size()))
			throw std::ios_base::failure("bad seek offset");

		pos = next;
		return pos;
	}
};

}
}


#endif
