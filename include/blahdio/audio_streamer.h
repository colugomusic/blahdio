#pragma once

#include <cstdint>
#include <memory>

namespace blahdio {

namespace impl { class AudioReader; class AudioStreamer; }

class AudioStreamer
{
public:

	AudioStreamer(std::shared_ptr<impl::AudioReader> reader);
	AudioStreamer(const AudioStreamer&) = delete;
	AudioStreamer(AudioStreamer&& rhs) noexcept;
	~AudioStreamer();

	// Return value < frames_to_read indicates the end of the stream
	std::uint32_t read_frames(void* buffer, std::uint32_t frames_to_read);
	bool seek(std::uint64_t frame);

private:

	impl::AudioStreamer* impl_;
};

}
