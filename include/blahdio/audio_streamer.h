#pragma once

#include <cstdint>

namespace blahdio {

namespace impl { class AudioReader; class AudioStreamer; }

class AudioStreamer
{
public:

	AudioStreamer(impl::AudioReader* reader);
	AudioStreamer(const AudioStreamer&) = delete;
	AudioStreamer(AudioStreamer&& rhs) noexcept;
	~AudioStreamer();

	// Return value < frames_to_read indicates the end of the stream
	std::uint32_t read_frames(void* buffer, std::uint32_t frames_to_read);

private:

	impl::AudioStreamer* impl_;
};

}
