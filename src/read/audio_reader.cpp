#include <stdexcept>
#include <vector>
#include "binary/binary_reader.h"
#include "typed_read_handler.h"

namespace blahdio {
namespace impl {

class AudioReader
{
public:

	AudioReader(const std::string& utf8_path, AudioType type_hint);
	AudioReader(const blahdio::AudioReader::Stream& stream, AudioType type_hint);
	AudioReader(const void* data, std::size_t data_size, AudioType type_hint);

	void set_binary_frame_size(int frame_size);

	void read_header();
	void read_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size);

	const AudioDataFormat& get_format() const { return format_; }

	AudioType get_type() const { return active_typed_handler_.type; }

private:

	read::typed::Handlers typed_handlers_;
	read::typed::Handler active_typed_handler_;
	read::binary::Handler binary_handler_;

	AudioType type_hint_ = AudioType::None;
	AudioDataFormat format_;
	int binary_frame_size_ = 1;

	void make_file_handlers(const std::string& utf8_path);
	void make_stream_handlers(const blahdio::AudioReader::Stream& stream);
	void make_memory_handlers(const void* data, std::size_t data_size);

	void read_binary_header();
	void read_typed_header();

	void read_binary_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size);
	void read_typed_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size);
};

void AudioReader::make_file_handlers(const std::string& utf8_path)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			binary_handler_ = read::binary::make_handler(utf8_path);
			break;
		}

		default:
		{
			typed_handlers_ = read::typed::make_handlers(utf8_path);
			break;
		}
	}
}

void AudioReader::make_stream_handlers(const blahdio::AudioReader::Stream& stream)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			binary_handler_ = read::binary::make_handler(stream);
			break;
		}

		default:
		{
			typed_handlers_ = read::typed::make_handlers(stream);
			break;
		}
	}
}

void AudioReader::make_memory_handlers(const void* data, std::size_t data_size)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			binary_handler_ = read::binary::make_handler(data, data_size);
			break;
		}

		default:
		{
			typed_handlers_ = read::typed::make_handlers(data, data_size);
			break;
		}
	}
}

void AudioReader::read_binary_header()
{
	binary_handler_.read_header(binary_frame_size_, &format_);
}

void AudioReader::read_typed_header()
{
	const auto type_handlers_to_try = typed_handlers_.make_type_attempt_order(type_hint_);

	for (auto type_handler : type_handlers_to_try)
	{
		AudioDataFormat format;

		if (type_handler && type_handler.try_read_header(&format))
		{
			active_typed_handler_ = type_handler;
			format_ = format;
			return;
		}
	}

	throw std::runtime_error("File format not recognized");
}

void AudioReader::read_binary_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size)
{
	binary_handler_.read_frames(callbacks, binary_frame_size_, chunk_size);
}

void AudioReader::read_typed_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size)
{
	// If the header hasn't been read yet, read it now
	if (active_typed_handler_.type == AudioType::None)
	{
		// Will throw if the audio type couldn't be deduced
		read_typed_header();
	}

	active_typed_handler_.read_frames(callbacks, format_, chunk_size);
}

AudioReader::AudioReader(const std::string& utf8_path, AudioType type_hint)
	: type_hint_(type_hint)
{
	make_file_handlers(utf8_path);
}

AudioReader::AudioReader(const blahdio::AudioReader::Stream& stream, AudioType type_hint)
	: type_hint_(type_hint)
{
	make_stream_handlers(stream);
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioType type_hint)
	: type_hint_(type_hint)
{
	make_memory_handlers(data, data_size);
}

void AudioReader::set_binary_frame_size(int frame_size)
{
	binary_frame_size_ = frame_size;
}

void AudioReader::read_header()
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			read_binary_header();
			break;
		}

		default:
		{
			read_typed_header();
			break;
		}
	}
}

void AudioReader::read_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			read_binary_frames(callbacks, chunk_size);
			break;
		}

		default:
		{
			read_typed_frames(callbacks, chunk_size);
			break;
		}
	}
}

} // impl

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

}
