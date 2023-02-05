#pragma once

#include <optional>
#include <variant>
#include <tl/expected.hpp>
#include "binary/binary_reader.h"
#include "typed_read_handler.h"

namespace blahdio {
namespace impl {

class AudioReader
{
public:

	AudioReader(std::string utf8_path, AudioTypeHint type_hint);
	AudioReader(const blahdio::AudioReader::Stream& stream, AudioTypeHint type_hint);
	AudioReader(const void* data, std::size_t data_size, AudioTypeHint type_hint);

	auto set_binary_frame_size(uint32_t frame_size) -> void;

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
		uint32_t binary_frame_size{1};
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

	struct BinaryHandler
	{
		read::binary::Handler handler;
		std::optional<AudioDataFormat> format{};

		[[nodiscard]] auto get_type() const -> expected<AudioType> { return AudioType::binary; }
		[[nodiscard]] auto read_header(Hints hints) -> expected<AudioDataFormat>;
		[[nodiscard]] auto read_frames(Hints hints, blahdio::AudioReader::Callbacks callbacks, uint32_t chunk_size) -> expected<void>;
		[[nodiscard]] auto stream_open(Hints hints) -> expected<AudioDataFormat>;
		[[nodiscard]] auto stream_close() -> expected<void>;
		[[nodiscard]] auto stream_read_frames(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>;
		[[nodiscard]] auto stream_seek(uint64_t frame) -> expected<void>;
	};

	using Handler = std::variant<TypedHandler, BinaryHandler>;

	Hints hints_;
	Handler handler_;

	[[nodiscard]] static
	auto make_file_handler(AudioTypeHint type_hint, std::string utf8_path) -> Handler;

	[[nodiscard]] static
	auto make_stream_handler(AudioTypeHint type_hint, const blahdio::AudioReader::Stream& stream) -> Handler;
	
	[[nodiscard]] static
	auto make_memory_handler(AudioTypeHint type_hint, const void* data, size_t data_size) -> Handler;
};

} // impl
} // blahdio
