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
		return std::uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static void read_stream_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
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
	typed::Handler out;

	out.type = AudioType::WAV;

	out.try_read_header = [utf8_path](AudioDataFormat* format)
	{
		drwav wav;

		if (!dr_libs::wav::init_file(&wav, utf8_path)) return false;

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	};

	out.read_frames = [utf8_path](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!dr_libs::wav::init_file(&wav, utf8_path)) throw std::runtime_error("Read error");

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	out.stream_open = [&out, utf8_path](AudioDataFormat* format)
	{
		auto wav = new drwav;

		if (!dr_libs::wav::init_file(wav, utf8_path))
		{
			delete wav;
			return false;
		}

		*format = get_header_info(wav);

		out.stream = wav;

		return true;
	};

	out.stream_read = [&out](void* buffer, std::uint32_t frames_to_read)
	{
		return drwav_read_pcm_frames_f32((drwav*)(out.stream), frames_to_read, (float*)(buffer));
	};

	out.stream_close = [&out]()
	{
		const auto wav = (drwav*)(out.stream);
		drwav_uninit(wav);
		delete wav;
	};

	return out;
}

// Stream
typed::Handler make_handler(const AudioReader::Stream& stream)
{
	typed::Handler out;

	out.type = AudioType::WAV;

	out.try_read_header = [stream](AudioDataFormat* format) -> bool
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

	out.read_frames = [stream](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!drwav_init(&wav, drwav_stream_read, drwav_stream_seek, (void*)(&stream), nullptr))
		{
			throw std::runtime_error("Read error");
		}

		read_stream_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	out.stream_open = [&out, stream](AudioDataFormat* format)
	{
		auto wav = new drwav;

		if (!drwav_init(wav, drwav_stream_read, drwav_stream_seek, (void*)(&stream), nullptr))
		{
			delete wav;
			return false;
		}

		*format = get_header_info(wav);

		out.stream = wav;

		return true;
	};

	out.stream_read = [&out](void* buffer, std::uint32_t frames_to_read)
	{
		return drwav_read_pcm_frames_f32((drwav*)(out.stream), frames_to_read, (float*)(buffer));
	};

	out.stream_close = [&out]()
	{
		const auto wav = (drwav*)(out.stream);
		drwav_uninit(wav);
		delete wav;
	};

	return out;
}

// Memory
typed::Handler make_handler(const void* data, std::size_t data_size)
{
	typed::Handler out;

	out.type = AudioType::WAV;

	out.try_read_header = [data, data_size](AudioDataFormat* format)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, data, data_size, nullptr)) return false;

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	};

	out.read_frames = [data, data_size](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, data, data_size, nullptr)) throw std::runtime_error("Read error");

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	};

	out.stream_open = [&out, data, data_size](AudioDataFormat* format)
	{
		auto wav = new drwav;

		if (!drwav_init_memory(wav, data, data_size, nullptr))
		{
			delete wav;
			return false;
		}

		*format = get_header_info(wav);

		out.stream = wav;

		return true;
	};

	out.stream_read = [&out](void* buffer, std::uint32_t frames_to_read)
	{
		return drwav_read_pcm_frames_f32((drwav*)(out.stream), frames_to_read, (float*)(buffer));
	};

	out.stream_close = [&out]()
	{
		const auto wav = (drwav*)(out.stream);
		drwav_uninit(wav);
		delete wav;
	};

	return out;
}

std::vector<typed::Handler> make_attempt_order(const typed::Handlers& handlers)
{
	std::vector<typed::Handler> out;

	out.push_back(handlers.wav);

#	if BLAHDIO_ENABLE_MP3
	out.push_back(handlers.mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
	out.push_back(handlers.flac);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
	out.push_back(handlers.wavpack);
#	endif

	return out;
}

}}}