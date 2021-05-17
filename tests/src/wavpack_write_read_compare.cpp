#include <catch2/catch.hpp>
#include <cmath>
#include "util.h"

SCENARIO("WavPack files can be written and read back with no errors", "[wavpack]")
{
	static constexpr auto NUM_FRAMES = 44100;
	static constexpr auto NUM_CHANNELS = 2;
	static constexpr auto SAMPLE_RATE = 44100;
	static constexpr auto BIT_DEPTH = 32;
	static constexpr auto WRITE_CHUNK_SIZE = 512;
	static constexpr auto READ_CHUNK_SIZE = 512;
	
	static const auto FILE_TEST_WV = std::filesystem::path(DIR_TEST_FILES) / "test.wv";

	GIVEN("A buffer of 2-channel audio data")
	{
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

		WHEN("The data is written as an 8-bit WavPack file")
		{
			const auto bit_depth = 8;

			util::write_wavpack(FILE_TEST_WV, write_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

			AND_WHEN("The data is read back")
			{
				std::vector<float> read_buffer(NUM_FRAMES * NUM_CHANNELS);

				util::read_wavpack(FILE_TEST_WV, read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

				THEN("The buffer written is the same as the buffer read back")
				{
					const auto tolerance = 1.0f / ((1 << (bit_depth - 1)) - 1);

					util::compare_frames(write_buffer.data(), read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, tolerance);
				}
			}
		}

		WHEN("The data is written as a 16-bit WavPack file")
		{
			const auto bit_depth = 16;

			util::write_wavpack(FILE_TEST_WV, write_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

			AND_WHEN("The data is read back")
			{
				std::vector<float> read_buffer(NUM_FRAMES * NUM_CHANNELS);

				util::read_wavpack(FILE_TEST_WV, read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

				THEN("The buffer written is the same as the buffer read back")
				{
					const auto tolerance = 1.0f / ((1 << (bit_depth - 1)) - 1);

					util::compare_frames(write_buffer.data(), read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, tolerance);
				}
			}
		}

		WHEN("The data is written as a 24-bit WavPack file")
		{
			const auto bit_depth = 24;

			util::write_wavpack(FILE_TEST_WV, write_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

			AND_WHEN("The data is read back")
			{
				std::vector<float> read_buffer(NUM_FRAMES * NUM_CHANNELS);

				util::read_wavpack(FILE_TEST_WV, read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

				THEN("The buffer written is the same as the buffer read back")
				{
					const auto tolerance = 1.0f / ((1 << (bit_depth - 1)) - 1);

					util::compare_frames(write_buffer.data(), read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, tolerance);
				}
			}
		}

		WHEN("The data is written as a 32-bit WavPack file")
		{
			const auto bit_depth = 32;

			util::write_wavpack(FILE_TEST_WV, write_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

			AND_WHEN("The data is read back")
			{
				std::vector<float> read_buffer(NUM_FRAMES * NUM_CHANNELS);

				util::read_wavpack(FILE_TEST_WV, read_buffer.data(), NUM_FRAMES, NUM_CHANNELS, SAMPLE_RATE, bit_depth);

				THEN("The buffer written is the same as the buffer read back")
				{
					util::compare_frames(write_buffer.data(), read_buffer.data(), NUM_FRAMES, NUM_CHANNELS);
				}
			}
		}
	}
}
