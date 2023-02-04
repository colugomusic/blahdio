#include "audio_streamer_impl.h"
#include "audio_reader_impl.h"

namespace blahdio {
namespace impl {

AudioStreamer::AudioStreamer(std::shared_ptr<impl::AudioReader> reader)
	: reader_(reader)
{
}

auto AudioStreamer::read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return reader_->stream_read_frames(buffer, frames_to_read);
}

auto AudioStreamer::seek(uint64_t frame) -> expected<void>
{
	return reader_->stream_seek(frame);
}

auto AudioStreamer::open() -> expected<AudioDataFormat>
{
	return reader_->stream_open();
}

auto AudioStreamer::close() -> expected<void>
{
	return reader_->stream_close();
}

} // impl
} // blahdio
