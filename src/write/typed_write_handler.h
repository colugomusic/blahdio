#pragma once

#include <array>

#include "blahdio/audio_writer.h"

namespace blahdio {
namespace write {
namespace typed {

struct Handler
{
	using WriteFunc = std::function<void(AudioWriter::Callbacks, std::uint32_t)>;

	WriteFunc write;
};

}}}