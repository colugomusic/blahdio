#pragma once

#include <functional>
#include <string>
#include "blahdio/audio_data_format.h"
#include "blahdio/audio_type.h"

namespace blahdio {

namespace impl { class AudioWriter; }

class AudioWriter
{
public:

	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using GetNextChunkFunc = std::function<void(float*, std::uint64_t, std::uint32_t)>;

		ShouldAbortFunc should_abort;
		GetNextChunkFunc get_next_chunk;
	};

	struct Stream
	{
		enum class SeekOrigin { Start, Current };

		using SeekFunc = std::function<bool(SeekOrigin, std::int64_t)>;

		// Returns the number of bytes read from the buffer
		// Returning a value < bytes_to_write indicates the end of the stream.
		using WriteBytesFunc = std::function<std::uint32_t(const void* data, std::uint32_t bytes_to_write)>;

		SeekFunc seek; // Not needed for WavPack writing
		WriteBytesFunc write_bytes;
	};

	// Write to file
	AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format);

	// Write to stream
	AudioWriter(const Stream& stream, AudioType type, const AudioDataFormat& format);

	~AudioWriter();

	void write_frames(Callbacks callbacks, std::uint32_t chunk_size);

private:

	impl::AudioWriter* impl_;
};

}