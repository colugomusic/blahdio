#pragma once

#include "blahdio/audio_reader.h"
#include "read/generic_reader.h"

namespace blahdio {
namespace read {
namespace binary {

struct Handler
{
	using ReadHeaderFunc = std::function<expected<AudioDataFormat>(int frame_size)>;
	using ReadFramesFunc = std::function<expected<void>(AudioReader::Callbacks, int frame_size, uint32_t chunk_size)>;

	using StreamOpenFunc = std::function<expected<AudioDataFormat>(int frame_size)>;
	using StreamReadFunc = std::function<expected<uint32_t>(void* buffer, uint32_t frames_to_read)>;
	using StreamCloseFunc = std::function<expected<void>()>;

	ReadHeaderFunc read_header;
	ReadFramesFunc read_frames;

	void* stream;

	StreamOpenFunc stream_open;
	StreamReadFunc stream_read;
	StreamCloseFunc stream_close;
};

class Reader : public GenericReader
{
	virtual void read_chunk(uint32_t num_frames, char* buffer) = 0;

public:

	virtual auto read_all_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void> override;
};

extern Handler make_handler(std::string utf8_path);
extern Handler make_handler(const AudioReader::Stream& stream);
extern Handler make_handler(const void* data, std::size_t data_size);

}}}