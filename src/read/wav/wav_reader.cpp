#include "wav_reader.h"
#include "blahdio/audio_writer.h"
#include "mackron/blahdio_dr_libs.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace wav {

static AudioDataFormat get_header_info(drwav* w)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = w->channels;
	out.num_frames = w->totalPCMFrameCount;
	out.sample_rate = w->sampleRate;
	out.bit_depth = w->bitsPerSample;

	return out;
}

static void read_frame_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return drwav_read_pcm_frames_f32(wav, read_size, buffer) == read_size;
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static AudioReader::Stream::SeekOrigin convert(drwav_seek_origin drwav_origin)
{
	switch (drwav_origin)
	{
		case drwav_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drwav_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static size_t drwav_stream_read(void* user_data, void* buffer, size_t bytes_to_read)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static drwav_bool32 drwav_stream_seek(void* user_data, int offset, drwav_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

// File
typed::Handler make_handler(const std::string& utf8_path)
{
	const auto try_read_header = [utf8_path](AudioDataFormat* format)
	{
		drwav wav;

		if (!dr_libs::wav::init_file(&wav, utf8_path)) return false;

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	};

	const auto read_frames = [utf8_path](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!dr_libs::wav::init_file(&wav, utf8_path)) throw std::runtime_error("Read error");

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	return { AudioType::WAV, try_read_header, read_frames };
}

// Stream
typed::Handler make_handler(const AudioReader::Stream& stream)
{
	const auto try_read_header = [stream](AudioDataFormat* format) -> bool
	{
		drwav wav;

		if (!drwav_init(&wav, drwav_stream_read, drwav_stream_seek, (void*)(&stream), nullptr))
		{
			return false;
		}

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	};

	const auto read_frames = [stream](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!drwav_init(&wav, drwav_stream_read, drwav_stream_seek, (void*)(&stream), nullptr))
		{
			throw std::runtime_error("Read error");
		}

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	return { AudioType::WAV, try_read_header, read_frames };
}

// Memory
typed::Handler make_handler(const void* data, std::size_t data_size)
{
	const auto try_read_header = [data, data_size](AudioDataFormat* format)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, data, data_size, nullptr)) return false;

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	};

	const auto read_frames = [data, data_size](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, data, data_size, nullptr)) throw std::runtime_error("Read error");

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	return { AudioType::WAV, try_read_header, read_frames };
}

}
}
}