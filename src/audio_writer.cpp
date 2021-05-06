#include <fstream>
#include <stdexcept>
#include <vector>
#include <wavpack.h>
#include <utf8.h>

#include "blahdio/audio_writer.h"
#include "dr_libs/dr_libs_utils.h"
#include "miniaudio/blahdio_miniaudio.h"

namespace blahdio {

void AudioWriter::write(Callbacks callbacks, std::uint32_t chunk_size)
{
	type_handler_.write(callbacks, chunk_size);
}

static AudioWriter::StreamWriter::SeekOrigin convert(drwav_seek_origin drwav_origin)
{
	switch (drwav_origin)
	{
		case drwav_seek_origin_start: return AudioWriter::StreamWriter::SeekOrigin::Start;
		case drwav_seek_origin_current: default: return AudioWriter::StreamWriter::SeekOrigin::Current;
	}
}

static size_t drwav_stream_write(void* user_data, const void* data, size_t bytes_to_write)
{
	const auto stream = (AudioWriter::StreamWriter*)(user_data);

	return stream->write_bytes(data, bytes_to_write);
}

static drwav_bool32 drwav_stream_seek(void* user_data, int offset, drwav_seek_origin origin)
{
	const auto stream = (AudioWriter::StreamWriter*)(user_data);

	return stream->seek(convert(origin), offset);
}

static drwav_data_format make_drwav_format(const AudioDataFormat& format)
{
	drwav_data_format out;

	out.container = drwav_container_riff;
	out.format = DR_WAVE_FORMAT_PCM;
	out.channels = format.num_channels;
	out.sampleRate = format.sample_rate;
	out.bitsPerSample = format.bit_depth;

	return out;
}

static ma_format get_miniaudio_pcm_format(int bit_depth)
{
	switch (bit_depth)
	{
		case 8: return ma_format_u8;
		case 16: return ma_format_s16;
		case 24: return ma_format_s24;
		case 32: default: return ma_format_s32;
	}
}

static drwav_uint64 drwav_write_f32_pcm_frames(drwav* wav, size_t write_size, int num_channels, const float* frames, int bit_depth)
{
	std::vector<char> buffer(size_t(bit_depth / 8) * num_channels * write_size);
		
	ma_convert_pcm_frames_format(buffer.data(), get_miniaudio_pcm_format(bit_depth), frames, ma_format_f32, write_size, num_channels, ma_dither_mode_triangle);

	return drwav_write_pcm_frames(wav, write_size, buffer.data());
}

static void drwav_write_frames(drwav* wav, AudioWriter::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	std::uint64_t frame = 0;

	while (frame < format.num_frames)
	{
		if (callbacks.should_abort && callbacks.should_abort()) break;

		auto write_size = chunk_size;

		if (frame + write_size >= format.num_frames)
		{
			write_size = format.num_frames - frame;
		}

		std::vector<float> interleaved_frames(size_t(write_size) * format.num_channels);

		callbacks.get_next_chunk(interleaved_frames.data(), frame, write_size);

		if (drwav_write_f32_pcm_frames(wav, write_size, format.num_channels, interleaved_frames.data(), format.bit_depth) != write_size)
		{
			throw std::runtime_error("Write error");
		}

		frame += write_size;

		if (callbacks.report_progress) callbacks.report_progress(frame);
	}
}

static AudioWriter::TypeHandler make_wav_handler(const AudioWriter::StreamWriter& stream, const AudioDataFormat& format)
{
	const auto write_func = [stream, format](AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
	{
		drwav wav;

		auto drwav_format = make_drwav_format(format);

		if (!drwav_init_write(&wav, &drwav_format, drwav_stream_write, drwav_stream_seek, (void*)(&stream), nullptr))
		{
			throw std::runtime_error("Failed to open WAV stream for writing.");
		}

		try
		{
			drwav_write_frames(&wav, callbacks, format, chunk_size);
		}
		catch (const std::exception& err)
		{
			drwav_uninit(&wav);
			throw err;
		}

		drwav_uninit(&wav);
	};

	return { write_func };
}

static AudioWriter::TypeHandler make_wav_handler(const std::string& utf8_path, const AudioDataFormat& format)
{
	const auto write_func = [utf8_path, format](AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
	{
		drwav wav;

		const auto drwav_format = make_drwav_format(format);

		if (!dr_libs::wav::init_file_write(&wav, utf8_path, &drwav_format))
		{
			throw std::runtime_error("Failed to open WAV file for writing.");
		}

		try
		{
			drwav_write_frames(&wav, callbacks, format, chunk_size);
		}
		catch (const std::exception& err)
		{
			drwav_uninit(&wav);
			throw err;
		}

		drwav_uninit(&wav);
	};

	return { write_func };
}

static void wavpack_write_file(WavpackBlockOutput blockout, void* id, const AudioDataFormat& format, AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
{
	const auto context = WavpackOpenFileOutput(blockout, id, nullptr);

	constexpr auto CFG_MONO = 4;
	constexpr auto CFG_STEREO = 3;

	WavpackConfig config = { 0 };

	config.bytes_per_sample = format.bit_depth / 8;
	config.bits_per_sample = format.bit_depth;
	config.channel_mask = format.num_channels == 1 ? CFG_MONO : CFG_STEREO;
	config.num_channels = format.num_channels;
	config.sample_rate = format.sample_rate;

	if (!WavpackSetConfiguration64(context, &config, format.num_frames, nullptr))
	{
		throw std::runtime_error(WavpackGetErrorMessage(context));
	}

	if (!WavpackPackInit(context))
	{
		throw std::runtime_error(WavpackGetErrorMessage(context));
	}

	std::uint64_t frame = 0;

	while (frame < format.num_frames)
	{
		if (callbacks.should_abort && callbacks.should_abort()) break;

		auto write_size = chunk_size;

		if (frame + write_size >= format.num_frames)
		{
			write_size = format.num_frames - frame;
		}

		std::vector<float> interleaved_frames(size_t(write_size) * format.num_channels);

		callbacks.get_next_chunk(interleaved_frames.data(), frame, write_size);

		std::vector<std::int32_t> samples(size_t(write_size) * format.num_channels);

		const auto scale = (1 << (format.bit_depth - 1)) - 1;

		for (int i = 0; i < samples.size(); i++)
		{
			samples[i] = std::int32_t(interleaved_frames[i] * scale);
		}

		if (!WavpackPackSamples(context, samples.data(), write_size))
		{
			throw std::runtime_error("Write error");
		}

		frame += write_size;

		if (callbacks.report_progress) callbacks.report_progress(frame);
	}

	if (!WavpackFlushSamples(context))
	{
		throw std::runtime_error("Write error");
	}

	WavpackCloseFile(context);
}

static AudioWriter::TypeHandler make_wavpack_handler(const AudioWriter::StreamWriter& stream, const AudioDataFormat& format)
{
	const auto write_func = [stream, format](AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
	{
		const auto blockout = [](void* id, void* data, int32_t bcount) -> int
		{
			auto stream = (AudioWriter::StreamWriter*)(id);

			if (stream->write_bytes(data, bcount) != bcount) return 0;

			return 1;
		};

		wavpack_write_file(blockout, (void*)(&stream), format, callbacks, chunk_size);
	};

	return { write_func };
}

static AudioWriter::TypeHandler make_wavpack_handler(const std::string& utf8_path, const AudioDataFormat& format)
{
	const auto write_func = [utf8_path, format](AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
	{
		const auto blockout = [](void* id, void* data, int32_t bcount) -> int
		{
			auto file = (std::ofstream*)(id);

			file->write((const char*)(data), bcount);
			
			if (file->fail()) return 0;

			return 1;
		};

		std::ofstream file;

#ifdef _WIN32
		file.open((const wchar_t*)(utf8::utf8to16(utf8_path).c_str()), std::fstream::binary);
#else
        file.open(utf8_path, std::fstream::binary);
#endif
		
		wavpack_write_file(blockout, &file, format, callbacks, chunk_size);
	};

	return { write_func };
}

static AudioWriter::TypeHandler make_type_handler(const AudioWriter::StreamWriter& stream, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
		case AudioType::WavPack: return make_wavpack_handler(stream, format);
		case AudioType::WAV: default: return make_wav_handler(stream, format);
	}
}

static AudioWriter::TypeHandler make_type_handler(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
		case AudioType::WavPack: return make_wavpack_handler(utf8_path, format);
		case AudioType::WAV: default: return make_wav_handler(utf8_path, format);
	}
}

AudioWriter::AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
	: type_handler_(make_type_handler(utf8_path, type, format))
{
}

AudioWriter::AudioWriter(const StreamWriter& stream, AudioType type, const AudioDataFormat& format)
	: type_handler_(make_type_handler(stream, type, format))
{
}

}
