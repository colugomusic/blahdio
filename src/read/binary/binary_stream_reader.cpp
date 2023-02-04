#include "binary_stream_reader.h"
#include <vector>

namespace blahdio {
namespace read {
namespace binary {

StreamReader::StreamReader(const AudioReader::Stream& stream, int frame_size)
	: stream_{ stream }
{
	frame_size_ = frame_size;
	num_channels_ = 1;
}

auto StreamReader::read_all_frames(Callbacks callbacks, std::uint32_t chunk_size) -> expected<void>
{
	std::uint64_t frame = 0;

	for (;;)
	{
		if (callbacks.should_abort()) break;

		std::vector<char> frame_data(size_t(chunk_size) * frame_size_);

		const auto frames_read = read_chunk(chunk_size, frame_data.data());

		callbacks.return_chunk((const void*)(frame_data.data()), frame, chunk_size);

		if (frames_read < chunk_size) break;

		frame += frames_read;
	}

	return {};
}

std::uint32_t StreamReader::read_chunk(std::uint32_t num_frames, char* buffer)
{
	const auto read_size = num_frames * frame_size_;
	const auto frames_read = stream_.read_bytes(buffer, read_size);

	num_frames_ += frames_read;

	return frames_read;
}

}}}