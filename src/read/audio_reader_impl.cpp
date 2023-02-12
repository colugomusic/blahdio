#include "audio_reader_impl.h"
#include <stdexcept>

namespace blahdio {
namespace impl {

auto AudioReader::make_file_handler(std::string utf8_path) -> TypedHandler
{
	return TypedHandler{read::typed::make_handlers(std::move(utf8_path))};
}

auto AudioReader::make_stream_handler(const blahdio::AudioReader::Stream& stream) -> TypedHandler
{
	return TypedHandler{read::typed::make_handlers(stream)};
}

auto AudioReader::make_memory_handler(const void* data, size_t data_size) -> TypedHandler
{
	return TypedHandler{read::typed::make_handlers(data, data_size)};
}

AudioReader::AudioReader(std::string utf8_path, AudioTypeHint type_hint)
	: handler_{make_file_handler(std::move(utf8_path))}
{
	hints_.type = type_hint;
}

AudioReader::AudioReader(const blahdio::AudioReader::Stream& stream, AudioTypeHint type_hint)
	: handler_{make_stream_handler(stream)}
{
	hints_.type = type_hint;
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioTypeHint type_hint)
	: handler_{make_memory_handler(data, data_size)}
{
	hints_.type = type_hint;
}

auto AudioReader::get_format() const -> expected<AudioDataFormat>
{
	if (!handler_.format)
	{
		return tl::make_unexpected("Failed to get audio data format (The header has not been read yet)");
	}

	return *handler_.format;
}

auto AudioReader::get_type() const -> expected<AudioType>
{
	return handler_.get_type();
}

[[nodiscard]] auto AudioReader::read_header() -> expected<AudioDataFormat>
{
	return handler_.read_header(hints_);
}

auto AudioReader::read_frames(blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	const auto read_header_if_not_already_read_yet = [&]() -> expected<void>
	{
		if (!handler_.format)
		{
			auto result{handler_.read_header(hints_)};

			if (!result)
			{
				return tl::make_unexpected(result.error());
			}
		}

		return {};
	};

	const auto read_frames = [&]() -> expected<void>
	{
		return handler_.read_frames(hints_, callbacks, chunk_size);
	};

	return read_header_if_not_already_read_yet().and_then(read_frames);
}

auto AudioReader::stream_open() -> expected<AudioDataFormat>
{
	return handler_.stream_open(hints_);
}

auto AudioReader::stream_close() -> expected<void>
{
	return handler_.stream_close();
}

auto AudioReader::stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return handler_.stream_read_frames(buffer, frames_to_read);
}

auto AudioReader::stream_seek(uint64_t frame) -> expected<void>
{
	return handler_.stream_seek(frame);
}

auto AudioReader::TypedHandler::get_type() const -> expected<AudioType>
{
	if (!active_handler)
	{
		return tl::make_unexpected("Failed to get audio type (The header has not been read yet)");
	}

	return active_handler->type();
}

auto AudioReader::TypedHandler::read_header(Hints hints) -> expected<AudioDataFormat>
{
	const auto type_handlers_to_try = handlers.make_type_attempt_order(hints.type);

	for (auto type_handler : type_handlers_to_try)
	{
		auto result{type_handler->try_read_header()};

		if (result)
		{
			active_handler = type_handler;
			format = *result;
			return *format;
		}
	}

	return tl::make_unexpected("File format not recognized");
}

auto AudioReader::TypedHandler::read_frames(Hints, blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	return active_handler->read_frames(callbacks, *format, chunk_size);
}

auto AudioReader::TypedHandler::stream_open(Hints hints) -> expected<AudioDataFormat>
{
	const auto type_handlers_to_try{handlers.make_type_attempt_order(hints.type)};

	for (auto type_handler : type_handlers_to_try)
	{
		auto open_result{type_handler->stream_open()};

		if (open_result)
		{
			active_handler = type_handler;
			format = *open_result;
			return *format;
		}
	}

	return tl::make_unexpected("File format not recognized");
}

auto AudioReader::TypedHandler::stream_close() -> expected<void>
{
	return active_handler->stream_close();
}

auto AudioReader::TypedHandler::stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return active_handler->stream_read(buffer, frames_to_read);
}

auto AudioReader::TypedHandler::stream_seek(uint64_t frame) -> expected<void>
{
	return active_handler->stream_seek(frame);
}

} // impl
} // blahdio
