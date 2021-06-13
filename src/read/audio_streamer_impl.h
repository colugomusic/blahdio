#pragma once

#include <cstdint>

namespace blahdio {
namespace impl {

class AudioReader;
class AudioStreamer
{
public:

	AudioStreamer(impl::AudioReader* reader);

	std::uint32_t read_frames(void* buffer, std::uint32_t frames_to_read);
	void open();
	void close();

private:

	impl::AudioReader* reader_;
};

} // impl
} // blahdio
