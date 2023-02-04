#include "blahdio/audio_streamer.h"
#include "audio_streamer_impl.h"

namespace blahdio {

AudioStreamer::AudioStreamer(std::shared_ptr<impl::AudioReader> reader)
	: impl_(new impl::AudioStreamer(reader))
{
	impl_->open();
}

AudioStreamer::AudioStreamer(AudioStreamer&& rhs) noexcept
	: impl_(rhs.impl_)
{
	rhs.impl_ = nullptr;
}

AudioStreamer::~AudioStreamer()
{
	if (impl_)
	{
		impl_->close();
		delete impl_;
	}
}

auto AudioStreamer::read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return impl_->read_frames(buffer, frames_to_read);
}

auto AudioStreamer::seek(uint64_t frame) -> expected<void>
{
	return impl_->seek(frame);
}

}