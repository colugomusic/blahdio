#pragma once

#include "blahdio/audio_writer.h"

namespace blahdio {
namespace write {
namespace typed {

struct Handler
{
	using WriteFramesFunc = std::function<void(AudioWriter::Callbacks, std::uint32_t)>;

	WriteFramesFunc write_frames;
};

extern Handler make_handler(const AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format);
extern Handler make_handler(const std::string& utf8_path, AudioType type, const AudioDataFormat& format);

}}}