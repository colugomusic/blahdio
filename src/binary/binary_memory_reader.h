#pragma once

#include "binary_reader.h"

namespace blahdio {
namespace binary {

class MemoryReader : public Reader
{
	const char* data_;
	std::uint64_t read_pos_ = 0;

	void read_chunk(std::uint32_t num_frames, char* buffer) override;

public:

	MemoryReader(int frame_size, const void* data, std::size_t data_size);
};

}}