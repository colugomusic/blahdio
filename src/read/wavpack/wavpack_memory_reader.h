#pragma once

#include "wavpack_reader.h"
#include <wavpack.h>

namespace blahdio {
namespace read {
namespace wavpack {

class MemoryReader : public Reader
{
	WavpackContext* open() override;

	struct Stream
	{
		const void* data;
		std::size_t data_size;
		std::int64_t pos = 0;
		unsigned char ungetc_char;
		bool ungetc_flag = false;
	} stream_;

	WavpackStreamReader64 stream_reader_;

	void init_stream_reader();

public:

	MemoryReader(const void* data, std::size_t data_size);
};

}}}