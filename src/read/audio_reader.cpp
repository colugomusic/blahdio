#include "audio_reader_impl.h"

namespace blahdio {

AudioReader::AudioReader(const std::string& utf8_path, AudioType type_hint)
	: impl_(new impl::AudioReader(utf8_path, type_hint))
{
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioType type_hint)
	: impl_(new impl::AudioReader(data, data_size, type_hint))
{
}

AudioReader::~AudioReader()
{
	delete impl_;
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

AudioStreamer AudioReader::stream()
{
	return AudioStreamer(impl_);
}

} // blahdio
