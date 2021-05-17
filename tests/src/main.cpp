#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <blahdio/audio_reader.h>
#include <blahdio/audio_writer.h>
#include <cmath>
#include <filesystem>

using namespace blahdio;

TEST_CASE("32-bit wavpack float overflow", "[wavpack]")
{
	static constexpr auto NUM_FRAMES = 44100 * 10;
	static constexpr auto NUM_CHANNELS = 2;
	static constexpr auto SAMPLE_RATE = 44100;
	static constexpr auto BIT_DEPTH = 32;
	static constexpr auto WRITE_CHUNK_SIZE = 512;
	static constexpr auto READ_CHUNK_SIZE = 512;
	static constexpr auto VALUE_CHECK_TOLERANCE = 0.000001f;
	
	static const std::filesystem::path DIR_TEST_FILES("test_files");
	static const auto FILE_TEST_WV = DIR_TEST_FILES / "test.wv";

	AudioDataFormat write_format;

	write_format.bit_depth = BIT_DEPTH;
	write_format.num_channels = NUM_CHANNELS;
	write_format.num_frames = NUM_FRAMES;
	write_format.sample_rate = SAMPLE_RATE;

	std::vector<float> write_buffer(NUM_FRAMES * NUM_CHANNELS);

	double phase = 0.0;

	for (int i = 0; i < NUM_FRAMES; i++)
	{
		float o = std::sin(phase);
		phase += 3.14159*2 * 256 / 44100.0;

		for (int c = 0; c < NUM_CHANNELS; c++)
		{
			write_buffer[(i * NUM_CHANNELS) + c] = o;
		}
	}

	std::filesystem::create_directory(DIR_TEST_FILES);
	AudioWriter writer(FILE_TEST_WV, AudioType::WavPack, write_format);

	AudioWriter::Callbacks writer_callbacks;

	writer_callbacks.should_abort = []() { return false; };

	writer_callbacks.get_next_chunk = [&write_buffer](float* buffer, std::uint64_t frame, std::uint32_t num_frames)
	{
		for(int i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < NUM_CHANNELS; c++)
			{
				buffer[(i * NUM_CHANNELS) + c] = write_buffer[((frame + i) * NUM_CHANNELS) + c];
			}
		}
	};

	writer.write_frames(writer_callbacks, WRITE_CHUNK_SIZE);

	AudioReader reader(FILE_TEST_WV, AudioType::WavPack);

	reader.read_header();

	const auto num_frames = reader.get_num_frames();
	const auto num_channels = reader.get_num_channels();
	const auto sample_rate = reader.get_sample_rate();
	const auto bit_depth = reader.get_bit_depth();

	REQUIRE(num_frames == write_format.num_frames);
	REQUIRE(num_channels == write_format.num_channels);
	REQUIRE(sample_rate == write_format.sample_rate);
	REQUIRE(bit_depth == write_format.bit_depth);

	std::vector<float> read_buffer(NUM_FRAMES * NUM_CHANNELS);

	blahdio::AudioReader::Callbacks reader_callbacks;

	reader_callbacks.should_abort = []() { return false; };

	reader_callbacks.return_chunk = [&read_buffer](const void* data, std::uint64_t frame, std::uint32_t num_frames)
	{
		for (int i = 0; i < num_frames; i++)
		{
			for (int c = 0; c < NUM_CHANNELS; c++)
			{
				read_buffer[((frame + i) * NUM_CHANNELS) + c] = ((const float*)(data))[(i * NUM_CHANNELS) + c];
			}
		}
	};

	reader.read_frames(reader_callbacks, READ_CHUNK_SIZE);

	for (int i = 0; i < NUM_FRAMES; i++)
	{
		for (int c = 0; c < NUM_CHANNELS; c++)
		{
			const auto write_value = write_buffer[(i * NUM_CHANNELS) + c];
			const auto read_value = read_buffer[(i * NUM_CHANNELS) + c];
			const auto diff = std::abs(write_value - read_value);

			INFO("Frame: " << i << " Channel: " << c);
			INFO("Write value was " << write_value << " but read value was " << read_value);
			REQUIRE(diff < VALUE_CHECK_TOLERANCE);
		}
	}
}
