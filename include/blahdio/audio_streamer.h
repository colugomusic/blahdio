#pragma once

#include <cstdint>
#include <memory>
#include "blahdio/expected.h"

namespace blahdio {

namespace impl { class AudioReader; class AudioStreamer; }

class AudioStreamer
{
public:

	AudioStreamer();
	AudioStreamer(AudioStreamer&&);
	auto operator=(AudioStreamer&&) -> AudioStreamer&;
	AudioStreamer(std::shared_ptr<impl::AudioReader> reader);
	~AudioStreamer();

	// Return value < frames_to_read indicates the end of the stream
	auto read_frames(void* buffer, std::uint32_t frames_to_read) -> expected<uint32_t>;
	auto seek(std::uint64_t frame) -> expected<void>;

private:

	std::unique_ptr<impl::AudioStreamer> impl_;
};

}
