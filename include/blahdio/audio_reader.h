#pragma once

#include <functional>
#include <memory>
#include <string>
#include "audio_data_format.h"
#include "audio_type.h"
#include "expected.h"

namespace blahdio {

namespace impl { class AudioReader; }

class AudioStreamer;

class AudioReader
{
public:

	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using ReturnChunkFunc = std::function<void(const void* data, uint64_t first_frame_index, uint32_t num_frames)>;

		ShouldAbortFunc should_abort;
		ReturnChunkFunc return_chunk;
	};

	struct Stream
	{
		enum class SeekOrigin { Start, Current };

		using SeekFunc = std::function<bool(SeekOrigin origin, int64_t offset)>;

		// Returns the number of bytes written to the buffer
		// Returning a value < bytes_to_read indicates the end of the stream.
		using ReadBytesFunc = std::function<uint32_t(void* buffer, uint32_t bytes_to_read)>;

		SeekFunc seek; // Not needed for WavPack reading
		ReadBytesFunc read_bytes;
	};
	
	// Read from file
	AudioReader(std::string utf8_path, AudioTypeHint type_hint);

	// Read from stream
	AudioReader(const Stream& stream, AudioTypeHint type_hint);

	// Read from memory
	AudioReader(const void* data, size_t data_size, AudioTypeHint type_hint);

	// Size in bytes of each frame to return when reading AudioType::Binary data
	auto set_binary_frame_size(int frame_size) -> void;

	// Create a streamer
	[[nodiscard]] auto streamer() -> AudioStreamer;

	// Just read the header
	[[nodiscard]] auto read_header() -> expected<AudioDataFormat>;

	// Read all of the frames
	// If the header has not been read yet, it will be read automatically here
	[[nodiscard]] auto read_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void>;

	// Header must be read first before calling these
	[[nodiscard]] auto get_format() const -> expected<AudioDataFormat>;
	[[nodiscard]] auto get_type() const -> expected<AudioType>;

private:

	std::shared_ptr<impl::AudioReader> impl_;
};

}
