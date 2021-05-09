#include "mp3_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include "read/generic_dr_libs_reader.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace mp3 {

static AudioDataFormat get_header_info(drmp3* mp3)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = mp3->channels;
	out.num_frames = drmp3_get_pcm_frame_count(mp3);
	out.sample_rate = mp3->sampleRate;
	out.bit_depth = 32;

	return out;
}

static void read_frame_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [mp3](float* buffer, std::uint32_t read_size)
	{
		return drmp3_read_pcm_frames_f32(mp3, read_size, buffer) == read_size;
	};

	generic_dr_libs_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

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