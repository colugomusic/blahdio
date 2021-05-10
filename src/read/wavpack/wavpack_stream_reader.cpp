#include "wavpack_stream_reader.h"

namespace blahdio {
namespace read {
namespace wavpack {

void StreamReader::init_stream_reader()
{
	stream_reader_.can_seek = [](void* id) -> int
	{
		return 0;
	};

	stream_reader_.close = [](void* id) -> int
	{
		return true;
	};

	stream_reader_.get_length = [](void* id) -> std::int64_t
	{
		return 0;
	};

	stream_reader_.get_pos = [](void* id) -> std::int64_t
	{
		return 0;
	};

	stream_reader_.push_back_byte = [](void* id, int c) -> int
	{
		const auto stream = (Stream*)(id);

		stream->pos--;

		return c;
	};

	stream_reader_.read_bytes = [](void* id, void* data, std::int32_t bcount) -> std::int32_t
	{
		const auto stream = (Stream*)(id);

		if (bcount < 1) return 0;

		if (stream->ungetc_flag)
		{
			*((char*)(data)) = stream->ungetc_char;
			stream->ungetc_flag = false;
			bcount--;
		}

		if (bcount < 1) return 0;

		return std::int32_t(stream->client_stream.read_bytes(data, bcount));
	};

	stream_reader_.set_pos_abs = [](void* id, std::int64_t pos) -> int
	{
		return 0;
	};

	stream_reader_.set_pos_rel = [](void* id, std::int64_t delta, int mode) -> int
	{
		return 0;
	};

	stream_reader_.truncate_here = nullptr;
	stream_reader_.write_bytes = nullptr;
}

StreamReader::StreamReader(const AudioReader::Stream& stream)
{
	init_stream_reader();

	stream_.client_stream = stream;
}

WavpackContext* StreamReader::open()
{
	int flags = 0;

	flags |= OPEN_2CH_MAX;
	flags |= OPEN_NORMALIZE;
	flags |= OPEN_STREAMING;

	char error[80];

	return WavpackOpenFileInputEx64(&stream_reader_, &stream_, nullptr, error, flags, 0);
}

}
}
}