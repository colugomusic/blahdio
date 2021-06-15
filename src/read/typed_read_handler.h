#pragma once

#include <memory>
#include <vector>

#include "blahdio/audio_data_format.h"
#include "blahdio/audio_reader.h"

namespace blahdio {
namespace read {
namespace typed {

struct Handler
{
	virtual AudioType type() const { return AudioType::None; }

	virtual bool try_read_header(AudioDataFormat* format) = 0;
	virtual void read_frames(AudioReader::Callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) = 0;

	virtual bool stream_open(AudioDataFormat* format) = 0;
	virtual std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) = 0;
	virtual void stream_close() = 0;
};

struct Handlers
{
#	if BLAHDIO_ENABLE_FLAC
		std::shared_ptr<Handler> flac;
#	endif

#	if BLAHDIO_ENABLE_MP3
		std::shared_ptr<Handler> mp3;
#	endif

#	if BLAHDIO_ENABLE_WAV
		std::shared_ptr<Handler> wav;
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		std::shared_ptr<Handler> wavpack;
#	endif

	std::vector<std::shared_ptr<read::typed::Handler>> make_type_attempt_order(AudioType type) const;
};

extern Handlers make_handlers(const std::string& utf8_path);
extern Handlers make_handlers(const AudioReader::Stream& stream);
extern Handlers make_handlers(const void* data, std::size_t data_size);

}}}
