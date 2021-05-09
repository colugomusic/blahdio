#include "generic_dr_libs_reader.h"
#include <stdexcept>
#include <vector>

namespace blahdio {
namespace read {

void generic_dr_libs_frame_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<bool(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels,
	std::uint64_t num_frames)
{
	std::uint64_t frame = 0;

	while (frame < num_frames)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		auto read_size = chunk_size;

		if (frame + read_size >= num_frames)
		{
			read_size = std::uint32_t(num_frames - frame);
		}

		interleaved_frames.resize(size_t(read_size) * num_channels);

		if (!read_func(interleaved_frames.data(), read_size)) throw std::runtime_error("Read error");

		callbacks.return_chunk(frame, (const void*)(interleaved_frames.data()), read_size);

		frame += read_size;
	}
}

}}