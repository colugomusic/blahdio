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

	blahdio::AudioDataFormat format;

	format.num_frames = NUM_FRAMES;
	format.num_channels = NUM_CHANNELS;
	format.storage_type = blahdio::AudioDataFormat::StorageType::Int;

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
						format.bit_depth = bit_depth;
						format.sample_rate = sample_rate;

						util::write_read_compare(data.buffer.data(), audio_type, format);
					}
				}
			}
		}
	}

	format.bit_depth = 32;

	for (const auto & data : DATA)
	{
		GIVEN(data.description)
		{
			for (auto sample_rate : SAMPLE_RATES)
			{
				format.sample_rate = sample_rate;

				GIVEN("WavPack storage type: Normalized float")
				{
					format.storage_type = blahdio::AudioDataFormat::StorageType::NormalizedFloat;

					util::write_read_compare(data.buffer.data(), blahdio::AudioType::WavPack, format);
				}

				GIVEN("WavPack storage type: Unnormalized float")
				{
					format.storage_type = blahdio::AudioDataFormat::StorageType::Float;

					util::write_read_compare(data.buffer.data(), blahdio::AudioType::WavPack, format);
				}
			}
		}
	}
}
