#pragma once

#include "frame_data.h"

namespace blahdio {

template <class Input, class T, class Allocator>
void deinterleave(const Input& src, std::uint64_t read_frame_begin, std::uint64_t read_frame_end, FrameData<T, Allocator>& dst, std::uint64_t write_frame_begin)
{
	const auto num_channels = dst.get_num_channels();

	auto read_frame = read_frame_begin;
	auto write_frame = write_frame_begin;

	while (read_frame != read_frame_end)
	{
		for (int c = 0; c < num_channels; c++)
		{
			dst[c][write_frame] = src[(read_frame * num_channels) + c];
		}

		read_frame++;
		write_frame++;
	}
}

template <class T, class Allocator, class Output>
void interleave(const FrameData<T, Allocator>& src, std::uint64_t read_frame_begin, std::uint64_t read_frame_end, Output& dst, std::uint64_t write_frame_begin)
{
	const auto num_channels = src.get_num_channels();

	auto read_frame = read_frame_begin;
	auto write_frame = write_frame_begin;

	while (read_frame != read_frame_end)
	{
		for (int c = 0; c < num_channels; c++)
		{
			dst[(write_frame * num_channels) + c] = src[c][read_frame];
		}

		read_frame++;
		write_frame++;
	}
}

}