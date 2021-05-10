#include "wav_writer.h"
#include "mackron/blahdio_dr_libs.h"
#include <miniaudio.h>
#include <stdexcept>

namespace blahdio {
namespace write {
namespace wav {

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

static AudioWriter::Stream::SeekOrigin convert(drwav_seek_origin drwav_origin)
{
	switch (drwav_origin)
	{
		case drwav_seek_origin_start: return AudioWriter::Stream::SeekOrigin::Start;
		case drwav_seek_origin_current: default: return AudioWriter::Stream::SeekOrigin::Current;
	}
}

static size_t drwav_stream_write(void* user_data, const void* data, size_t bytes_to_write)
{
	const auto stream = (AudioWriter::Stream*)(user_data);

	return stream->write_bytes(data, std::uint32_t(bytes_to_write));
}

static drwav_bool32 drwav_stream_seek(void* user_data, int offset, drwav_seek_origin origin)
{
	const auto stream = (AudioWriter::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
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
			write_size = std::uint32_t(format.num_frames - frame);
		}

		std::vector<float> interleaved_frames(size_t(write_size) * format.num_channels);

		callbacks.get_next_chunk(interleaved_frames.data(), frame, write_size);

		if (drwav_write_f32_pcm_frames(wav, write_size, format.num_channels, interleaved_frames.data(), format.bit_depth) != write_size)
		{
			throw std::runtime_error("Write error");
		}

		frame += write_size;
	}
}

typed::Handler make_handler(const AudioWriter::Stream& stream, const AudioDataFormat& format)
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

typed::Handler make_handler(const std::string& utf8_path, const AudioDataFormat& format)
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

}}}