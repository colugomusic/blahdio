#include "flac_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include "read/generic_dr_libs_reader.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace flac {

static AudioDataFormat get_header_info(drflac* flac)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = flac->channels;
	out.num_frames = flac->totalPCMFrameCount;
	out.sample_rate = flac->sampleRate;
	out.bit_depth = flac->bitsPerSample;

	return out;
}

static void read_frame_data(drflac* flac, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [flac](float* buffer, std::uint32_t read_size)
	{
		return drflac_read_pcm_frames_f32(flac, read_size, buffer) == read_size;
	};

	generic_dr_libs_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

typed::Handler make_handler(const std::string& utf8_path)
{
	const auto try_read_header = [utf8_path](AudioDataFormat* format) -> bool
	{
		auto flac = dr_libs::flac::open_file(utf8_path);

		if (!flac) return false;

		*format = get_header_info(flac);

		drflac_close(flac);

		return true;
	};

	const auto read_frames = [utf8_path](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		auto flac = dr_libs::flac::open_file(utf8_path);

		if (!flac) throw std::runtime_error("Read error");

		read_frame_data(flac, callbacks, format, chunk_size);

		drflac_close(flac);
	};

	return { AudioType::FLAC, try_read_header, read_frames };
}

typed::Handler make_handler(const void* data, std::size_t data_size)
{
	const auto try_read_header = [data, data_size](AudioDataFormat* format) -> bool
	{
		auto flac = drflac_open_memory(data, data_size, nullptr);

		if (!flac) return false;

		*format = get_header_info(flac);

		drflac_close(flac);

		return true;
	};

	const auto read_frames = [data, data_size](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		auto flac = drflac_open_memory(data, data_size, nullptr);

		if (!flac) throw std::runtime_error("Read error");

		read_frame_data(flac, callbacks, format, chunk_size);

		drflac_close(flac);
	};

	return { AudioType::FLAC, try_read_header, read_frames };
}

}
}
}