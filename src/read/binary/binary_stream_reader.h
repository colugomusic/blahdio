#pragma once

#include "binary_reader.h"

namespace blahdio {
namespace read {
namespace binary {

class StreamReader : public GenericReader
{
public:

	StreamReader(const AudioReader::Stream& stream, int frame_size);

	auto read_all_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void> override;

private:

	std::uint32_t read_chunk(std::uint32_t num_frames, char* buffer);

	AudioReader::Stream stream_;
};

}
}
}