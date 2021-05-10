#pragma once

#include <array>

#include "blahdio/audio_data_format.h"
#include "blahdio/audio_reader.h"

namespace blahdio {
namespace read {
namespace typed {

struct Handler
{
	using TryReadHeaderFunc = std::function<bool(AudioDataFormat*)>;
	using ReadFramesFunc = std::function<void(AudioReader::Callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)>;

	AudioType type = AudioType::None;

	TryReadHeaderFunc try_read_header;
	ReadFramesFunc read_frames;

	operator bool() const { return type != AudioType::None; }
};

struct Handlers
{
	Handler flac;
	Handler mp3;
	Handler wav;
	Handler wavpack;

	std::array<read::typed::Handler, 4> make_type_attempt_order(AudioType type) const;
};

extern Handlers make_handlers(const std::string& utf8_path);
extern Handlers make_handlers(const AudioReader::Stream& stream);
extern Handlers make_handlers(const void* data, std::size_t data_size);

}}}