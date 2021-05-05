#include "binary_reader.h"

namespace blahdio {
namespace binary {

void Reader::read_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	std::uint64_t frame = 0;

	while (frame < num_frames_)
	{
		if (callbacks.should_abort()) break;

		auto frames_to_read = chunk_size;

		if (frame + frames_to_read >= num_frames_)
		{
			frames_to_read = num_frames_ - frame;
		}

		const auto read_size = size_t(frames_to_read) * frame_size_;

		std::vector<char> frame_data(read_size);

		read_chunk(frames_to_read, frame_data.data());

		callbacks.return_chunk(frame, (const void*)(frame_data.data()), frames_to_read);

		frame += frames_to_read;
	}
}
}}