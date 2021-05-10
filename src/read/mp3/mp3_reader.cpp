#include "mp3_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace mp3 {

static AudioDataFormat get_header_info(drmp3* m)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = m->channels;
	out.num_frames = drmp3_get_pcm_frame_count(m);
	out.sample_rate = m->sampleRate;
	out.bit_depth = 32;

	return out;
}

static void read_frame_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [mp3](float* buffer, std::uint32_t read_size)
	{
		return drmp3_read_pcm_frames_f32(mp3, read_size, buffer) == read_size;
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static AudioReader::Stream::SeekOrigin convert(drmp3_seek_origin drmp3_origin)
{
	switch (drmp3_origin)
	{
		case drmp3_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drmp3_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static size_t drmp3_stream_read(void* user_data, void* buffer, size_t bytes_to_read)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static drmp3_bool32 drmp3_stream_seek(void* user_data, int offset, drmp3_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

// File
typed::Handler make_handler(const std::string& utf8_path)
{
	const auto try_read_header = [utf8_path](AudioDataFormat* format)
	{
		drmp3 mp3;

		if (!dr_libs::mp3::init_file(&mp3, utf8_path)) return false;

		*format = get_header_info(&mp3);

		drmp3_uninit(&mp3);

		return true;
	};

	const auto read_frames = [utf8_path](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drmp3 mp3;

		if (!dr_libs::mp3::init_file(&mp3, utf8_path)) throw std::runtime_error("Read error");

		read_frame_data(&mp3, callbacks, format, chunk_size);

		drmp3_uninit(&mp3);
	};

	return { AudioType::MP3, try_read_header, read_frames };
}

// Stream
typed::Handler make_handler(const AudioReader::Stream& stream)
{
	const auto try_read_header = [stream](AudioDataFormat* format) -> bool
	{
		drmp3 mp3;

		if (!drmp3_init(&mp3, drmp3_stream_read, drmp3_stream_seek, (void*)(&stream), nullptr))
		{
			return false;
		}

		*format = get_header_info(&mp3);

		drmp3_uninit(&mp3);

		return true;
	};

	const auto read_frames = [stream](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drmp3 mp3;

		if (!drmp3_init(&mp3, drmp3_stream_read, drmp3_stream_seek, (void*)(&stream), nullptr))
		{
			throw std::runtime_error("Read error");
		}

		read_frame_data(&mp3, callbacks, format, chunk_size);

		drmp3_uninit(&mp3);
	};

	return { AudioType::MP3, try_read_header, read_frames };
}

// Memory
typed::Handler make_handler(const void* data, std::size_t data_size)
{
	const auto try_read_header = [data, data_size](AudioDataFormat* format) -> bool
	{
		drmp3 mp3;

		if (!drmp3_init_memory(&mp3, data, data_size, nullptr)) return false;

		*format = get_header_info(&mp3);

		drmp3_uninit(&mp3);

		return true;
	};

	const auto read_frames = [data, data_size](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drmp3 mp3;

		if (!drmp3_init_memory(&mp3, data, data_size, nullptr)) throw std::runtime_error("Read error");

		read_frame_data(&mp3, callbacks, format, chunk_size);

		drmp3_uninit(&mp3);
	};

	return { AudioType::MP3, try_read_header, read_frames };
}

}
}
}