#include "util.h"
#include <cmath>
#include <catch2/catch.hpp>
#include <blahdio/audio_reader.h>
#include <blahdio/audio_writer.h>

using namespace blahdio;

namespace util {

void compare_frames(const float* const a, const float* const b, std::uint64_t num_frames, int num_channels, float tolerance)
{
	for (int i = 0; i < num_frames; i++)
	{
		for (int c = 0; c < num_channels; c++)
		{
			const auto a_value = a[(i * num_channels) + c];
			const auto b_value = b[(i * num_channels) + c];
			const auto diff = std::abs(a_value - b_value);

			INFO("Buffer comparison failed at frame " << i << ", channel " << c);
			INFO("value A: " << a_value << ", value B: " << b_value << ", diff: " << diff << ", tolerance: " << tolerance);

			REQUIRE(diff < tolerance);
		}
	}
}

void write_wavpack(
		const std::filesystem::path& file_path,
		const float* write_buffer,
		std::uint64_t num_frames,
		int num_channels,
		int sample_rate,
		int bit_depth,
		int chunk_size)
{
	AudioDataFormat write_format;

	write_format.num_frames = num_frames;
	write_format.num_channels = num_channels;
	write_format.sample_rate = sample_rate;
	write_format.bit_depth = bit_depth;

	const auto dir = file_path.parent_path();

	if (!std::filesystem::exists(dir))
	{
		std::filesystem::create_directory(dir);
	}

	AudioWriter writer(file_path, AudioType::WavPack, write_format);

	AudioWriter::Callbacks writer_callbacks;

	writer_callbacks.should_abort = []() { return false; };

	writer_callbacks.get_next_chunk = [write_buffer, num_channels](float* buffer, std::uint64_t frame, std::uint32_t num_frames)
	{
		for(int i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < num_channels; c++)
			{
				buffer[(i * num_channels) + c] = write_buffer[((frame + i) * num_channels) + c];
			}
		}
	};

	writer.write_frames(writer_callbacks, chunk_size);
}

void read_wavpack(
		const std::filesystem::path& file_path,
		float* read_buffer,
		std::uint64_t num_frames_expected,
		int num_channels_expected,
		int sample_rate_expected,
		int bit_depth_expected,
		int chunk_size)
{
	AudioReader reader(file_path, AudioType::WavPack);

	reader.read_header();

	const auto num_frames = reader.get_num_frames();
	const auto num_channels = reader.get_num_channels();
	const auto sample_rate = reader.get_sample_rate();
	const auto bit_depth = reader.get_bit_depth();

	REQUIRE(num_frames == num_frames_expected);
	REQUIRE(num_channels == num_channels_expected);
	REQUIRE(sample_rate == sample_rate_expected);
	REQUIRE(bit_depth == bit_depth_expected);

	blahdio::AudioReader::Callbacks reader_callbacks;

	reader_callbacks.should_abort = []() { return false; };

	reader_callbacks.return_chunk = [read_buffer, num_channels](const void* data, std::uint64_t frame, std::uint32_t num_frames)
	{
		for (int i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < num_channels; c++)
			{
				read_buffer[((frame + i) * num_channels) + c] = ((const float*)(data))[(i * num_channels) + c];
			}
		}
	};

	reader.read_frames(reader_callbacks, chunk_size);
}

} // util
