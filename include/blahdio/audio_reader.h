#pragma once

#include <functional>
#include <string>
#include "audio_data_format.h"
#include "audio_type.h"
#include "frame_data.h"

namespace blahdio {

class AudioReader
{
public:
	
	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using ReturnChunkFunc = std::function<void(std::uint64_t frame, const void* data, std::uint32_t size)>;

		ShouldAbortFunc should_abort;
		ReturnChunkFunc return_chunk;
	};

	// Read from file
	AudioReader(const std::string& utf8_path, AudioType type_hint = AudioType::None);

	// Read from memory
	AudioReader(const void* data, std::size_t data_size, AudioType type_hint = AudioType::None);

	// Size in bytes of each frame to return when reading using AudioType::Binary
	void set_binary_frame_size(std::size_t frame_size);

	void read_header();
	void read_frames(Callbacks callbacks, std::uint32_t chunk_size);

	int get_frame_size() const { return format_.frame_size; }
	int get_num_channels() const { return format_.num_channels; }
	std::uint64_t get_num_frames() const { return format_.num_frames; }
	int get_sample_rate() const { return format_.sample_rate; }
	int get_bit_depth() const { return format_.bit_depth; }
	AudioType get_type() const { return active_type_handler_.type; }

	struct TypeHandler
	{
		using TryReadHeaderFunc = std::function<bool(AudioDataFormat*)>;
		using ReadFramesFunc = std::function<void(Callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)>;

		AudioType type = AudioType::None;

		TryReadHeaderFunc try_read_header;
		ReadFramesFunc read_frames;

		operator bool() const { return type != AudioType::None; }
	};

	struct BinaryHandler
	{
		using ReadHeaderFunc = std::function<void(int frame_size, AudioDataFormat*)>;
		using ReadFramesFunc = std::function<void(Callbacks, int frame_size, std::uint32_t chunk_size)>;

		ReadHeaderFunc read_header;
		ReadFramesFunc read_frames;
	};

private:

	TypeHandler flac_handler_;
	TypeHandler mp3_handler_;
	TypeHandler wav_handler_;
	TypeHandler wavpack_handler_;
	TypeHandler active_type_handler_;
	BinaryHandler binary_handler_;

	AudioType type_hint_ = AudioType::None;
	AudioDataFormat format_;
	int binary_frame_size_ = 1;

	std::array<TypeHandler, 4> make_type_attempt_order(AudioType type);

	void make_file_handlers(const std::string& utf8_path);
	void make_memory_handlers(const void* data, std::size_t data_size);

	void make_binary_file_handler(const std::string& utf8_path);
	void make_binary_memory_handler(const void* data, std::size_t data_size);

	void make_typed_file_handlers(const std::string& utf8_path);
	void make_typed_memory_handlers(const void* data, std::size_t data_size);

	void read_binary_header();
	void read_typed_header();

	void read_binary_frames(Callbacks callbacks, std::uint32_t chunk_size);
	void read_typed_frames(Callbacks callbacks, std::uint32_t chunk_size);
};

}