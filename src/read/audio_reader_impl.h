#pragma once

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
	std::uint32_t read_frames(void* buffer, std::uint32_t frames_to_read);

	const AudioDataFormat& get_format() const { return format_; }

	AudioType get_type() const { return active_typed_handler_->type(); }

	void stream_open();
	void stream_close();

private:

	read::typed::Handlers typed_handlers_;
	std::shared_ptr<read::typed::Handler> active_typed_handler_;
	read::binary::Handler binary_handler_;

	AudioType type_hint_ = AudioType::None;
	AudioDataFormat format_;
	int binary_frame_size_ = 1;

	void make_file_handlers(const std::string& utf8_path);
	void make_stream_handlers(const blahdio::AudioReader::Stream& stream);
	void make_memory_handlers(const void* data, std::size_t data_size);

	void read_binary_header();
	void read_typed_header();
	void open_binary_stream();
	void open_typed_stream();
	void close_binary_stream();
	void close_typed_stream();

	void read_binary_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size);
	void read_typed_frames(blahdio::AudioReader::Callbacks callbacks, std::uint32_t chunk_size);
	std::uint32_t read_binary_frames(void* buffer, std::uint32_t frames_to_read);
	std::uint32_t read_typed_frames(void* buffer, std::uint32_t frames_to_read);
};

} // impl
} // blahdio
