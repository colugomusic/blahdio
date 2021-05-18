#include <catch2/catch.hpp>
#include <cmath>
#include "util.h"

SCENARIO("Data can be written and read back with no errors", "[wav][wavpack]")
{
	static constexpr auto NUM_FRAMES = 4410;
	static constexpr auto NUM_CHANNELS = 2;

	static constexpr blahdio::AudioType AUDIO_TYPES[] =
	{
		blahdio::AudioType::WAV,
		blahdio::AudioType::WavPack,
	};

	static constexpr int SAMPLE_RATES[] =
	{
		8000,
		11025,
		16000,
		22050,
		44100,
		48000,
		88200,
		96000,
		176400,
		192000,
		352800,
		384000,
	};
	
	static constexpr int BIT_DEPTHS[] =
	{
		8,
		16,
		24,
		32
	};

	struct Data
	{
		std::string description;
		std::vector<float> buffer;
	};

	const Data DATA[] =
	{
		{
			"256 frames of 2-channel sine data",
			util::generate_sine_data(NUM_FRAMES, NUM_CHANNELS, 256.0f),
		},
		{
			"256 frames of 2-channel noise data",
			util::generate_noise_data(NUM_FRAMES, NUM_CHANNELS),
		},
	};

	for (const auto & data : DATA)
	{
		GIVEN(data.description)
		{
			for (auto audio_type : AUDIO_TYPES)
			{
				for (auto sample_rate : SAMPLE_RATES)
				{
					for (auto bit_depth : BIT_DEPTHS)
					{
						util::write_read_compare(data.buffer.data(), audio_type, NUM_FRAMES, NUM_CHANNELS, sample_rate, bit_depth);
					}
				}
			}
		}
	}
}
