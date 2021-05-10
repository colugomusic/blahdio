#pragma once

#include "wavpack_reader.h"
#include <wavpack.h>

namespace blahdio {
namespace read {
namespace wavpack {

class StreamReader : public Reader
{
	WavpackContext* open() override;

	struct Stream
	{
		AudioReader::Stream client_stream;
		std::int64_t pos = 0;
		unsigned char ungetc_char;
		bool ungetc_flag = false;
	} stream_;

	WavpackStreamReader64 stream_reader_;

	void init_stream_reader();

public:

	StreamReader(const AudioReader::Stream& stream);
};

}
}
}