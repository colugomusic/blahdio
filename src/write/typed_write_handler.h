#pragma once

#include <array>

#include "blahdio/audio_writer.h"

namespace blahdio {
namespace write {
namespace typed {

struct Handler
{
	using WriteFramesFunc = std::function<void(AudioWriter::Callbacks, std::uint32_t)>;

	WriteFramesFunc write_frames;
};

}}}