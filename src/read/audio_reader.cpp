#include "audio_reader_impl.h"
#include "blahdio/audio_streamer.h"

namespace blahdio {

AudioReader::AudioReader(std::string utf8_path, AudioTypeHint type_hint)
	: impl_(std::make_shared<impl::AudioReader>(std::move(utf8_path), type_hint))
{
}

AudioReader::AudioReader(const void* data, size_t data_size, AudioTypeHint type_hint)
	: impl_(std::make_shared<impl::AudioReader>(data, data_size, type_hint))
{
}

AudioReader::AudioReader(const blahdio::AudioReader::Stream& stream, AudioTypeHint type_hint) 
	: impl_(std::make_shared<impl::AudioReader>(stream, type_hint))
{
}

void AudioReader::set_binary_frame_size(int frame_size)
{
	impl_->set_binary_frame_size(frame_size);
}

auto AudioReader::read_header() -> expected<AudioDataFormat>
{
	return impl_->read_header();
}

auto AudioReader::read_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	return impl_->read_frames(callbacks, chunk_size);
}

auto AudioReader::get_format() const -> expected<AudioDataFormat>
{
	return impl_->get_format();
}

auto AudioReader::get_type() const -> expected<AudioType>
{
	return impl_->get_type();
}

auto AudioReader::streamer() -> AudioStreamer
{
	return {impl_};
}

} // blahdio
