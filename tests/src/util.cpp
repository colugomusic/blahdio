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

void write_frames(
		const std::filesystem::path& file_path,
		const float* write_buffer,
		AudioType audio_type,
		AudioDataFormat write_format,
		int chunk_size)
{
	const auto dir = file_path.parent_path();

	if (!std::filesystem::exists(dir))
	{
		std::filesystem::create_directory(dir);
	}

	AudioWriter writer(file_path.string(), audio_type, write_format);

	AudioWriter::Callbacks writer_callbacks;

	writer_callbacks.should_abort = []() { return false; };

	writer_callbacks.get_next_chunk = [write_buffer, write_format](float* buffer, std::uint64_t frame, std::uint32_t num_frames)
	{
		for(std::uint32_t i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < write_format.num_channels; c++)
			{
				buffer[(i * write_format.num_channels) + c] = write_buffer[((frame + i) * write_format.num_channels) + c];
			}
		}
	};

	writer.write_frames(writer_callbacks, chunk_size);
}

void read_frames(
		const std::filesystem::path& file_path,
		float* read_buffer,
		AudioType audio_type_expected,
		AudioDataFormat format_expected,
		int chunk_size)
{
	AudioReader reader(file_path.string(), audio_type_expected);

	reader.read_header();

	const auto num_frames = reader.get_num_frames();
	const auto num_channels = reader.get_num_channels();
	const auto sample_rate = reader.get_sample_rate();
	const auto bit_depth = reader.get_bit_depth();
	const auto audio_type = reader.get_type();

	REQUIRE(num_frames == format_expected.num_frames);
	REQUIRE(num_channels == format_expected.num_channels);
	REQUIRE(sample_rate == format_expected.sample_rate);
	REQUIRE(bit_depth == format_expected.bit_depth);
	REQUIRE(audio_type == audio_type_expected);

	blahdio::AudioReader::Callbacks reader_callbacks;

	reader_callbacks.should_abort = []() { return false; };

	reader_callbacks.return_chunk = [read_buffer, num_channels](const void* data, std::uint64_t frame, std::uint32_t num_frames)
	{
		for (std::uint32_t i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < num_channels; c++)
			{
				read_buffer[((frame + i) * num_channels) + c] = ((const float*)(data))[(i * num_channels) + c];
			}
		}
	};

	reader.read_frames(reader_callbacks, chunk_size);
}

std::string to_string(AudioType audio_type)
{
	switch (audio_type)
	{
		case AudioType::None: return "None";
		case AudioType::Binary: return "Binary";
		case AudioType::FLAC: return "FLAC";
		case AudioType::MP3: return "MP3";
		case AudioType::WAV: return "WAV";
		case AudioType::WavPack: return "WavPack";
		default: return "Unknown";
	}
}

std::string get_ext(AudioType audio_type)
{
	switch (audio_type)
	{
		case AudioType::None: return "";
		case AudioType::Binary: return "bin";
		case AudioType::FLAC: return "flac";
		case AudioType::MP3: return "mp3";
		case AudioType::WAV: return "wav";
		case AudioType::WavPack: return "wv";
		default: return "Unknown";
	}
}

void write_read_compare(
		const float* data,
		blahdio::AudioType audio_type,
		blahdio::AudioDataFormat format,
		int chunk_size)
{
	WHEN("The data is written as a " << format.bit_depth << "-bit " << to_string(audio_type) << " @" << format.sample_rate << " file")
	{
		const auto test_file_name = std::string("test_") + std::to_string(format.bit_depth) + "_" + std::to_string(format.sample_rate);
		const auto test_file_path = (std::filesystem::path(DIR_TEST_FILES) / test_file_name).replace_extension(get_ext(audio_type));

		util::write_frames(test_file_path, data, audio_type, format);

		AND_WHEN("The data is read back")
		{
			std::vector<float> read_buffer(format.num_frames * format.num_channels);

			util::read_frames(test_file_path, read_buffer.data(), audio_type, format);

			THEN("The buffer written is the same as the buffer read back")
			{
				const auto tolerance = 1.0f / (1 << format.bit_depth / 2);

				util::compare_frames(data, read_buffer.data(), format.num_frames, format.num_channels, tolerance);
			}
		}
	}
}

std::vector<float> generate_sine_data(int num_frames, int num_channels, float frequency, int sample_rate)
{
	std::vector<float> out(num_frames * num_channels);

	double phase = 0.0;

	for (int i = 0; i < num_frames; i++)
	{
		float o = float(std::sin(phase));

		phase += 3.14159*2 * frequency / sample_rate;

		for (int c = 0; c < num_channels; c++)
		{
			out[(i * num_channels) + c] = o;
		}
	}

	return out;
}

std::vector<float> generate_noise_data(int num_frames, int num_channels)
{
	std::vector<float> out(num_frames * num_channels);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);

	for (int i = 0; i < num_frames * num_channels; i++)
	{
		out[i] = float(dis(gen));
	}

	return out;
}

} // util
