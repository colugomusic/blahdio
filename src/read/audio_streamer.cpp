#include "blahdio/audio_streamer.h"
#include "audio_streamer_impl.h"

namespace blahdio {

AudioStreamer::AudioStreamer() = default;
AudioStreamer::AudioStreamer(AudioStreamer&&) noexcept = default;
auto AudioStreamer::operator=(AudioStreamer&&) noexcept -> AudioStreamer& = default;

AudioStreamer::AudioStreamer(std::shared_ptr<impl::AudioReader> reader)
	: impl_{std::make_unique<impl::AudioStreamer>(reader)}
{
	impl_->open();
}

AudioStreamer::~AudioStreamer()
{
	if (impl_)
	{
		impl_->close();
	}
}

auto AudioStreamer::read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	if (!impl_)
	{
		return tl::make_unexpected("Can't read frames. The streamer is uninitialized.");
	}

	return impl_->read_frames(buffer, frames_to_read);
}

auto AudioStreamer::seek(uint64_t frame) -> expected<void>
{
	if (!impl_)
	{
		return tl::make_unexpected("Can't seek. The streamer is uninitialized.");
	}

	return impl_->seek(frame);
}

}