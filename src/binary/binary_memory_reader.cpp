#include "binary_memory_reader.h"
#include <cmath>

namespace blahdio {
namespace binary {

MemoryReader::MemoryReader(int frame_size, const void* data, std::size_t data_size)
	: data_((const char*)(data))
{
	frame_size_ = frame_size;
	num_channels_ = 1;
	num_frames_ = std::uint64_t(std::floor(float(data_size) / frame_size));
}

void MemoryReader::read_chunk(std::uint32_t num_frames, char* buffer)
{
	const auto read_size = num_frames * frame_size_;

	std::copy(data_ + read_pos_, data_ + read_pos_ + read_size, buffer);
}

}}
