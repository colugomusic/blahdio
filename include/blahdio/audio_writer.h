#pragma once

#include <functional>
#include <string>
#include "audio_data_format.h"
#include "audio_type.h"

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
		using WriteBytesFunc = std::function<std::uint32_t(const void*, std::uint32_t)>;

		SeekFunc seek; // Not needed for WavPack writing
		WriteBytesFunc write_bytes;

		operator bool() const { return seek && write_bytes; }
	};

	// Write to file
	AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format);

	// Write to stream
	AudioWriter(const Stream& stream, AudioType type, const AudioDataFormat& format);

	~AudioWriter();

	void write(Callbacks callbacks, std::uint32_t chunk_size);

private:

	impl::AudioWriter* impl_;
};

}