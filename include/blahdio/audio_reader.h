#pragma once

#include <functional>
#include <memory>
#include <string>
#include "audio_data_format.h"
#include "audio_type.h"

namespace blahdio {

namespace impl { class AudioReader; }

class AudioStreamer;

class AudioReader
{
public:

	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using ReturnChunkFunc = std::function<void(const void* data, std::uint64_t first_frame_index, std::uint32_t num_frames)>;

		ShouldAbortFunc should_abort;
		ReturnChunkFunc return_chunk;
	};

	struct Stream
	{
		enum class SeekOrigin { Start, Current };

		using SeekFunc = std::function<bool(SeekOrigin origin, std::int64_t offset)>;

		// Returns the number of bytes written to the buffer
		// Returning a value < bytes_to_read indicates the end of the stream.
		using ReadBytesFunc = std::function<std::uint32_t(void* buffer, std::uint32_t bytes_to_read)>;

		SeekFunc seek; // Not needed for WavPack reading
		ReadBytesFunc read_bytes;
	};
	
	// The reader will always read untyped binary data if type_hint == AudioType::Binary.
	// Otherwise each audio type will be tried, starting with type_hint first.
	
	// AudioType::Binary will never be deduced automatically, it must
	// be explicitly set here.

	// Read from file
	AudioReader(const std::string& utf8_path, AudioType type_hint = AudioType::None);

	// Read from stream
	AudioReader(const Stream& stream, AudioType type_hint = AudioType::None);

	// Read from memory
	AudioReader(const void* data, std::size_t data_size, AudioType type_hint = AudioType::None);

	// Size in bytes of each frame to return when reading AudioType::Binary data
	void set_binary_frame_size(int frame_size);

	void read_header();
	void read_frames(Callbacks callbacks, std::uint32_t chunk_size);

	int get_frame_size() const;
	int get_num_channels() const;

	// May return zero if source is a stream
	std::uint64_t get_num_frames() const;

	// These will always return zero when reading AudioType::Binary data
	int get_sample_rate() const;
	int get_bit_depth() const;

	AudioType get_type() const;

	std::shared_ptr<AudioStreamer> streamer();

private:

	std::shared_ptr<impl::AudioReader> impl_;
};

}
