#pragma once

#include "binary_reader.h"

namespace blahdio {
namespace read {
namespace binary {

class StreamReader : public Reader
{
public:

	StreamReader(const AudioReader::Stream& stream, int frame_size);

private:

	void read_chunk(std::uint32_t num_frames, char* buffer) override;

	AudioReader::Stream stream_;
};

}
}
}