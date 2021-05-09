#pragma once

#include "blahdio/audio_reader.h"
#include "read/generic_reader.h"

namespace blahdio {
namespace read {
namespace binary {

struct Handler
{
	using ReadHeaderFunc = std::function<void(int frame_size, AudioDataFormat*)>;
	using ReadFramesFunc = std::function<void(AudioReader::Callbacks, int frame_size, std::uint32_t chunk_size)>;

	ReadHeaderFunc read_header;
	ReadFramesFunc read_frames;
};

class Reader : public GenericReader
{
	virtual void read_chunk(std::uint32_t num_frames, char* buffer) = 0;

public:

	void read_frames(Callbacks callbacks, std::uint32_t chunk_size) override;
};

extern Handler make_handler(const std::string& utf8_path);
extern Handler make_handler(const void* data, std::size_t data_size);

}}}