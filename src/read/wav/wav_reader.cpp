#include "wav_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include "read/generic_dr_libs_reader.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace wav {

static AudioDataFormat get_header_info(drwav* wav)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = wav->channels;
	out.num_frames = wav->totalPCMFrameCount;
	out.sample_rate = wav->sampleRate;
	out.bit_depth = wav->bitsPerSample;

	return out;
}

static void read_frame_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return drwav_read_pcm_frames_f32(wav, read_size, buffer) == read_size;
	};

	generic_dr_libs_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

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