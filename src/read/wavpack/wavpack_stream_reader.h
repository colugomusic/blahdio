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

	void do_read_frames(Callbacks callbacks, std::uint32_t chunk_size, std::function<std::uint32_t(float* buffer, std::uint32_t read_size)> chunk_reader) override;

public:

	StreamReader(const AudioReader::Stream& stream);
};

}
}
}