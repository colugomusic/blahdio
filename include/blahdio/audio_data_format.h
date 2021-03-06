#pragma once

#include <cstdint>

namespace blahdio {

struct AudioDataFormat
{
	enum class StorageType
	{
		Default,
		Int,
		Float,
		NormalizedFloat,
	};

	int frame_size { 0 };
	int num_channels { 0 };
	std::uint64_t num_frames { 0 };
	int sample_rate { 0 };
	int bit_depth { 0 };

	StorageType storage_type { StorageType::Default };
};

}