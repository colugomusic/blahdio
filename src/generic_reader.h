#pragma once

#include <functional>

namespace blahdio {

class GenericReader
{
public:

	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using ReturnChunkFunc = std::function<void(std::uint64_t frame, const void* data, std::uint32_t size)>;

		ShouldAbortFunc should_abort;
		ReturnChunkFunc return_chunk;
	};

	int get_frame_size() const { return frame_size_; }
	int get_num_channels() const { return num_channels_; }
	std::uint64_t get_num_frames() const { return num_frames_; }
	int get_sample_rate() const { return sample_rate_; }
	int get_bit_depth() const { return bit_depth_; }

	virtual void read_frames(Callbacks callbacks, std::uint32_t chunk_size) = 0;

protected:

	int frame_size_ = 0;
	int num_channels_ = 0;
	std::uint64_t num_frames_ = 0;
	int sample_rate_ = 0;
	int bit_depth_ = 0;
};

}