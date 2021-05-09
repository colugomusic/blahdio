#pragma once

#include "blahdio/audio_reader.h"

namespace blahdio {
namespace read {

extern void generic_dr_libs_frame_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<bool(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels,
	std::uint64_t num_frames);

}}