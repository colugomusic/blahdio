#pragma once

#include <cstdint>
#include <filesystem>

namespace util {

extern void compare_frames(const float* const a, const float* const b, std::uint64_t num_frames, int num_channels, float tolerance = 0.000001f);

extern void write_wavpack(
		const std::filesystem::path& file_path,
		const float* buffer,
		std::uint64_t num_frames,
		int num_channels,
		int sample_rate,
		int bit_depth,
		int chunk_size = 512);

extern void read_wavpack(
		const std::filesystem::path& file_path,
		float* buffer,
		std::uint64_t num_frames_expected,
		int num_channels_expected,
		int sample_rate_expected,
		int bit_depth_expected,
		int chunk_size = 512);

} // util