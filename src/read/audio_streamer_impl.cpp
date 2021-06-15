#include "audio_streamer_impl.h"
#include "audio_reader_impl.h"

namespace blahdio {
namespace impl {

AudioStreamer::AudioStreamer(std::shared_ptr<impl::AudioReader> reader)
	: reader_(reader)
{
}

std::uint32_t AudioStreamer::read_frames(void* buffer, std::uint32_t frames_to_read)
{
	return reader_->stream_read_frames(buffer, frames_to_read);
}

bool AudioStreamer::skip_to_frame(std::uint64_t frame)
{
	return reader_->stream_seek(frame);
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
