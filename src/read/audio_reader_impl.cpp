#include "audio_reader_impl.h"
#include <stdexcept>

namespace blahdio {
namespace impl {

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

		if (type_handler && type_handler->try_read_header(&format))
		{
			active_typed_handler_ = type_handler;
			format_ = format;
			return;
		}
	}

	throw std::runtime_error("File format not recognized");
}

void AudioReader::open_binary_stream()
{
	binary_handler_.stream_open(binary_frame_size_, &format_);
}

void AudioReader::open_typed_stream()
{
	const auto type_handlers_to_try = typed_handlers_.make_type_attempt_order(type_hint_);

	for (auto type_handler : type_handlers_to_try)
	{
		AudioDataFormat format;

		if (type_handler && type_handler->stream_open(&format))
		{
			active_typed_handler_ = type_handler;
			format_ = format;
			return;
		}
	}

	throw std::runtime_error("File format not recognized");
}

void AudioReader::close_binary_stream()
{
	binary_handler_.stream_close();
}

void AudioReader::close_typed_stream()
{
	active_typed_handler_->stream_close();
}

void AudioReader::read_binary_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size)
{
	binary_handler_.read_frames(callbacks, binary_frame_size_, chunk_size);
}

void AudioReader::read_typed_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size)
{
	// If the header hasn't been read yet, read it now
	if (active_typed_handler_->type() == AudioType::None)
	{
		// Will throw if the audio type couldn't be deduced
		read_typed_header();
	}

	active_typed_handler_->read_frames(callbacks, format_, chunk_size);
}

void AudioReader::stream_open()
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			open_binary_stream();
			break;
		}

		default:
		{
			open_typed_stream();
			break;
		}
	}
}

void AudioReader::stream_close()
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			close_binary_stream();
			break;
		}

		default:
		{
			close_typed_stream();
			break;
		}
	}
}

std::uint32_t AudioReader::read_binary_frames(void* buffer, std::uint32_t frames_to_read)
{
	return binary_handler_.stream_read(buffer, frames_to_read);
}

std::uint32_t AudioReader::read_typed_frames(void* buffer, std::uint32_t frames_to_read)
{
	return active_typed_handler_->stream_read(buffer, frames_to_read);
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

std::uint32_t AudioReader::read_frames(void* buffer, std::uint32_t frames_to_read)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			return read_binary_frames(buffer, frames_to_read);
		}

		default:
		{
			return read_typed_frames(buffer, frames_to_read);
		}
	}
}

} // impl
} // blahdio
