#pragma once

#include <cstdint>
#include <memory>

namespace blahdio {
namespace impl {

class AudioReader;
class AudioStreamer
{
public:

	AudioStreamer(std::shared_ptr<impl::AudioReader> reader);

	std::uint32_t read_frames(void* buffer, std::uint32_t frames_to_read);
	bool seek(std::uint64_t frame);
	void open();
	void close();

private:

	std::shared_ptr<impl::AudioReader> reader_;
};

} // impl
} // blahdio
