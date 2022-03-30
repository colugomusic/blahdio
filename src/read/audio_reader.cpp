#include "audio_reader_impl.h"
#include "blahdio/audio_streamer.h"

namespace blahdio {

AudioReader::AudioReader(const std::string& utf8_path, AudioType type_hint)
	: impl_(std::make_shared<impl::AudioReader>(utf8_path, type_hint))
{
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioType type_hint)
	: impl_(std::make_shared<impl::AudioReader>(data, data_size, type_hint))
{
}

AudioReader::AudioReader(const blahdio::AudioReader::Stream& stream, AudioType type_hint) 
	: impl_(std::make_shared<impl::AudioReader>(stream, type_hint))
{
}

void AudioReader::set_binary_frame_size(int frame_size)
{
	impl_->set_binary_frame_size(frame_size);
}

void AudioReader::read_header()
{
	impl_->read_header();
}

void AudioReader::read_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	impl_->read_frames(callbacks, chunk_size);
}

int AudioReader::get_frame_size() const
{
	return impl_->get_format().frame_size;
}

int AudioReader::get_num_channels() const
{
	return impl_->get_format().num_channels;
}

std::uint64_t AudioReader::get_num_frames() const
{
	return impl_->get_format().num_frames;
}

int AudioReader::get_sample_rate() const
{
	return impl_->get_format().sample_rate;
}

int AudioReader::get_bit_depth() const
{
	return impl_->get_format().bit_depth;
}

AudioType AudioReader::get_type() const
{
	return impl_->get_type();
}

std::shared_ptr<AudioStreamer> AudioReader::streamer()
{
	return std::make_shared<AudioStreamer>(impl_);
}

} // blahdio
