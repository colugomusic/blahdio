#pragma once

#include <vector>

#include "blahdio/audio_data_format.h"
#include "blahdio/audio_reader.h"

namespace blahdio {
namespace read {
namespace typed {

struct Handler
{
	using TryReadHeaderFunc = std::function<bool(AudioDataFormat*)>;
	using ReadFramesFunc = std::function<void(AudioReader::Callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)>;

	using StreamOpenFunc = std::function<bool(AudioDataFormat*)>;
	using StreamReadFunc = std::function<std::uint32_t(void* buffer, std::uint32_t frames_to_read)>;
	using StreamCloseFunc = std::function<void()>;

	AudioType type = AudioType::None;

	TryReadHeaderFunc try_read_header;
	ReadFramesFunc read_frames;

	void* stream;

	StreamOpenFunc stream_open;
	StreamReadFunc stream_read;
	StreamCloseFunc stream_close;
		
	operator bool() const { return type != AudioType::None; }
};

struct Handlers
{
#	if BLAHDIO_ENABLE_FLAC
		Handler flac;
#	endif

#	if BLAHDIO_ENABLE_MP3
		Handler mp3;
#	endif

#	if BLAHDIO_ENABLE_WAV
		Handler wav;
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		Handler wavpack;
#	endif

	std::vector<read::typed::Handler> make_type_attempt_order(AudioType type) const;
};

extern Handlers make_handlers(const std::string& utf8_path);
extern Handlers make_handlers(const AudioReader::Stream& stream);
extern Handlers make_handlers(const void* data, std::size_t data_size);

}}}