#pragma once

#include <functional>
#include <string>
#include "audio_data_format.h"
#include "audio_type.h"
#include "frame_data.h"

namespace blahdio {

class AudioWriter
{
public:

	struct Callbacks
	{
		using ShouldAbortFunc = std::function<bool()>;
		using ReportProgressFunc = std::function<void(int frames_done)>;
		using GetNextChunkFunc = std::function<void(float*, int, std::uint32_t)>;

		ShouldAbortFunc should_abort;
		ReportProgressFunc report_progress;
		GetNextChunkFunc get_next_chunk;
	};

	struct StreamWriter
	{
		enum class SeekOrigin { Start, Current };

		using SeekFunc = std::function<bool(SeekOrigin, std::int64_t)>;
		using WriteBytesFunc = std::function<std::uint64_t(const void*, std::uint64_t)>;

		SeekFunc seek;
		WriteBytesFunc write_bytes;

		operator bool() const { return seek && write_bytes; }
	};

	AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format);
	AudioWriter(const StreamWriter& stream, AudioType type, const AudioDataFormat& format);

	void write(Callbacks callbacks, std::uint32_t chunk_size);

	struct TypeHandler
	{
		using WriteFunc = std::function<void(Callbacks, std::uint32_t)>;

		WriteFunc write;
	};

private:


	TypeHandler type_handler_;
};

}