#pragma once

#include <cstdint>
#include <memory>
#include "blahdio/audio_data_format.h"
#include "blahdio/expected.h"

namespace blahdio {
namespace impl {

class AudioReader;
class AudioStreamer
{
public:

	AudioStreamer(std::shared_ptr<impl::AudioReader> reader);

	auto read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>;
	auto seek(uint64_t frame) -> expected<void>;
	auto open() -> expected<AudioDataFormat>;
	auto close() -> expected<void>;

private:

	std::shared_ptr<impl::AudioReader> reader_;
};

} // impl
} // blahdio
