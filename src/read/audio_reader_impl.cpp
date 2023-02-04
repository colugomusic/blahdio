#include "audio_reader_impl.h"
#include <stdexcept>

namespace blahdio {
namespace impl {

auto AudioReader::make_file_handler(AudioType type_hint, std::string utf8_path) -> Handler
{
	switch (type_hint)
	{
		case AudioType::Binary:
		{
			return BinaryHandler{read::binary::make_handler(std::move(utf8_path))};
		}

		default:
		{
			return TypedHandler{read::typed::make_handlers(std::move(utf8_path))};
		}
	}
}

auto AudioReader::make_stream_handler(AudioType type_hint, const blahdio::AudioReader::Stream& stream) -> Handler
{
	switch (type_hint)
	{
		case AudioType::Binary:
		{
			return BinaryHandler{read::binary::make_handler(stream)};
		}

		default:
		{
			return TypedHandler{read::typed::make_handlers(stream)};
		}
	}
}

auto AudioReader::make_memory_handler(AudioType type_hint, const void* data, size_t data_size) -> Handler
{
	switch (type_hint)
	{
		case AudioType::Binary:
		{
			return BinaryHandler{read::binary::make_handler(data, data_size)};
		}

		default:
		{
			return TypedHandler{read::typed::make_handlers(data, data_size)};
		}
	}
}

AudioReader::AudioReader(std::string utf8_path, AudioType type_hint)
	: handler_{make_file_handler(type_hint, std::move(utf8_path))}
{
}

AudioReader::AudioReader(const blahdio::AudioReader::Stream& stream, AudioType type_hint)
	: handler_{make_stream_handler(type_hint, stream)}
{
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioType type_hint)
	: handler_{make_memory_handler(type_hint, data, data_size)}
{
}

void AudioReader::set_binary_frame_size(uint32_t frame_size)
{
	hints_.binary_frame_size = frame_size;
}

auto AudioReader::get_format() const -> expected<AudioDataFormat>
{
	return std::visit([=](auto&& handler) -> expected<AudioDataFormat>
	{
		if (!handler.format)
		{
			return tl::make_unexpected("Failed to get audio data format (The header has not been read yet)");
		}

		return *handler.format;
	}, handler_);
}

auto AudioReader::get_type() const -> expected<AudioType>
{
	return std::visit([=](auto&& handler) { return handler.get_type(); }, handler_);
}

[[nodiscard]] auto AudioReader::read_header() -> expected<AudioDataFormat>
{
	return std::visit([=](auto&& handler) { return handler.read_header(hints_); }, handler_);
}

auto AudioReader::read_frames(blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	return std::visit([=](auto&& handler)
	{
		const auto read_header_if_not_already_read_yet = [&]() -> expected<void>
		{
			if (!handler.format)
			{
				auto result{handler.read_header(hints_)};

				if (!result)
				{
					return tl::make_unexpected(result.error());
				}
			}

			return {};
		};

		const auto read_frames = [&]() -> expected<void>
		{
			return handler.read_frames(hints_, callbacks, chunk_size);
		};

		return read_header_if_not_already_read_yet().and_then(read_frames);
	}, handler_);
}

auto AudioReader::stream_open() -> expected<AudioDataFormat>
{
	return std::visit([=](auto&& handler)
	{
		return handler.stream_open(hints_);
	}, handler_);
}

auto AudioReader::stream_close() -> expected<void>
{
	return std::visit([=](auto&& handler) { return handler.stream_close(); }, handler_);
}

auto AudioReader::stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return std::visit([=](auto&& handler) { return handler.stream_read_frames(buffer, frames_to_read); }, handler_);
}

auto AudioReader::stream_seek(uint64_t frame) -> expected<void>
{
	return std::visit([=](auto&& handler) { return handler.stream_seek(frame); }, handler_);
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

auto AudioReader::BinaryHandler::read_header(Hints hints) -> expected<AudioDataFormat>
{
	auto result{handler.read_header(hints.binary_frame_size)};

	if (!result)
	{
		return tl::make_unexpected(result.error());
	}

	format = *result;
	return *format;
}

auto AudioReader::BinaryHandler::read_frames(Hints hints, blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	return handler.read_frames(callbacks, hints.binary_frame_size, chunk_size);
}

auto AudioReader::BinaryHandler::stream_open(Hints hints) -> expected<AudioDataFormat>
{
	auto result{handler.stream_open(hints.binary_frame_size)};

	if (!result)
	{
		return tl::make_unexpected(result.error());
	}

	format = *result;
	return *format;
}

auto AudioReader::BinaryHandler::stream_close() -> expected<void>
{
	return handler.stream_close();
}

auto AudioReader::BinaryHandler::stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
{
	return handler.stream_read(buffer, frames_to_read);
}

auto AudioReader::BinaryHandler::stream_seek(uint64_t frame) -> expected<void>
{
	return tl::make_unexpected("Can't seek binary data (Not implemented yet)");
}

} // impl
} // blahdio
