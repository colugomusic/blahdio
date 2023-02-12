#pragma once

#include <optional>
#include <variant>
#include <tl/expected.hpp>
#include "typed_read_handler.h"

namespace blahdio {
namespace impl {

class AudioReader
{
public:

	AudioReader(std::string utf8_path, AudioTypeHint type_hint);
	AudioReader(const blahdio::AudioReader::Stream& stream, AudioTypeHint type_hint);
	AudioReader(const void* data, std::size_t data_size, AudioTypeHint type_hint);

	[[nodiscard]] auto read_header() -> expected<AudioDataFormat>;
	[[nodiscard]] auto read_frames(blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>;
	[[nodiscard]] auto stream_open() -> expected<AudioDataFormat>;
	[[nodiscard]] auto stream_close() -> expected<void>;
	[[nodiscard]] auto stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>;
	[[nodiscard]] auto stream_seek(uint64_t frame) -> expected<void>;

	auto get_format() const -> expected<AudioDataFormat>;
	auto get_type() const -> expected<AudioType>;

private:

	struct Hints
	{
		AudioTypeHint type;
	};

	struct TypedHandler
	{
		read::typed::Handlers handlers;
		read::typed::Handler* active_handler{};
		std::optional<AudioDataFormat> format{};

		[[nodiscard]] auto get_type() const -> expected<AudioType>;
		[[nodiscard]] auto read_header(Hints hints) -> expected<AudioDataFormat>;
		[[nodiscard]] auto read_frames(Hints hints, blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>;
		[[nodiscard]] auto stream_open(Hints hints) -> expected<AudioDataFormat>;
		[[nodiscard]] auto stream_close() -> expected<void>;
		[[nodiscard]] auto stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>;
		[[nodiscard]] auto stream_seek(uint64_t frame) -> expected<void>;
	};

	Hints hints_;
	TypedHandler handler_;

	[[nodiscard]] static
	auto make_file_handler(std::string utf8_path) -> TypedHandler;

	[[nodiscard]] static
	auto make_stream_handler(const blahdio::AudioReader::Stream& stream) -> TypedHandler;
	
	[[nodiscard]] static
	auto make_memory_handler(const void* data, size_t data_size) -> TypedHandler;
};

} // impl
} // blahdio
