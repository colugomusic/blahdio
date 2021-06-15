#include "audio_streamer_impl.h"
#include "audio_reader_impl.h"

namespace blahdio {
namespace impl {

AudioStreamer::AudioStreamer(impl::AudioReader* reader)
	: reader_(reader)
{
}

std::uint32_t AudioStreamer::read_frames(void* buffer, std::uint32_t frames_to_read)
{
	return reader_->read_frames(buffer, frames_to_read);
}

void AudioStreamer::open()
{
	reader_->stream_open();
}

void AudioStreamer::close()
{
	reader_->stream_close();
}

} // impl
} // blahdio
