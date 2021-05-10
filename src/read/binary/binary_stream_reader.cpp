#include "binary_stream_reader.h"

namespace blahdio {
namespace read {
namespace binary {

StreamReader::StreamReader(const AudioReader::Stream& stream, int frame_size)
	: stream_{ stream }
{
	frame_size_ = frame_size;
	num_channels_ = 1;
}

void StreamReader::read_chunk(std::uint32_t num_frames, char* buffer)
{
	const auto read_size = num_frames * frame_size_;
	const auto frames_read = stream_.read_bytes(buffer, read_size);

	num_frames_ += frames_read;
}

}}}