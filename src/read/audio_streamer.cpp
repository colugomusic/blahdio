#include "blahdio/audio_streamer.h"
#include "audio_streamer_impl.h"

namespace blahdio {

AudioStreamer::AudioStreamer(impl::AudioReader* reader)
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

std::uint32_t AudioStreamer::read_frames(void* buffer, std::uint32_t frames_to_read)
{
	return impl_->read_frames(buffer, frames_to_read);
}

}