#pragma once

#include <string>
#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>
#include "blahdio/audio_reader.h"

namespace blahdio {
namespace dr_libs {

namespace flac {

extern drflac* open_file(const std::string& utf8_path);

}

namespace mp3 {

extern bool init_file(drmp3* mp3, const std::string& utf8_path);

}

namespace wav {

extern bool init_file(drwav* wav, const std::string& utf8_path);
extern bool init_file_write(drwav* wav, const std::string& utf8_path, const drwav_data_format* format);
}

extern void generic_frame_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<std::uint32_t(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels,
	std::uint64_t num_frames);

extern void generic_stream_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<std::uint32_t(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels);

}
}