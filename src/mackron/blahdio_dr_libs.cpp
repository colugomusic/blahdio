#include "blahdio_dr_libs.h"
#include <vector>
#include <utf8.h>

#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>

namespace blahdio {
namespace dr_libs {

namespace flac {

drflac* open_file(std::string_view utf8_path)
{
#ifdef _WIN32
	return drflac_open_file_w((const wchar_t*)(utf8::utf8to16(utf8_path).data()), nullptr);
#else
	return drflac_open_file(utf8_path.data(), nullptr);
#endif
}

} // flac

namespace mp3 {

bool init_file(drmp3* mp3, std::string_view utf8_path)
{
#ifdef _WIN32
	return drmp3_init_file_w(mp3, (const wchar_t*)(utf8::utf8to16(utf8_path).data()), nullptr);
#else
	return drmp3_init_file(mp3, utf8_path.data(), nullptr);
#endif
}

} // mp3

namespace wav {

bool init_file(drwav* wav, std::string_view utf8_path)
{
#ifdef _WIN32
	return drwav_init_file_w(wav, (const wchar_t*)(utf8::utf8to16(utf8_path).data()), nullptr);
#else
	return drwav_init_file(wav, utf8_path.data(), nullptr);
#endif
}

bool init_file_write(drwav* wav, std::string_view utf8_path, const drwav_data_format* format)
{
#ifdef _WIN32
	return drwav_init_file_write_w(wav, (const wchar_t*)(utf8::utf8to16(utf8_path).data()), format, nullptr);
#else
	return drwav_init_file_write(wav, utf8_path.data(), format, nullptr);
#endif
}

} // wav

[[nodiscard]] auto generic_frame_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<std::uint32_t(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels,
	std::uint64_t num_frames) -> tl::expected<void, std::string>
{
	std::uint64_t frame = 0;

	while (frame < num_frames)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		auto read_size = chunk_size;

		if (frame + read_size >= num_frames)
		{
			read_size = std::uint32_t(num_frames - frame);
		}

		interleaved_frames.resize(size_t(read_size) * num_channels);

		const auto frames_read = read_func(interleaved_frames.data(), read_size);
		
		callbacks.return_chunk((const void*)(interleaved_frames.data()), frame, frames_read);

		if (frames_read < read_size)
		{
			return tl::make_unexpected("Read error");
		}

		frame += frames_read;
	}

	return {};
}

void generic_stream_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<std::uint32_t(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels)
{
	std::uint64_t frame = 0;

	for (;;)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		interleaved_frames.resize(size_t(chunk_size) * num_channels);

		const auto frames_read = read_func(interleaved_frames.data(), chunk_size);

		callbacks.return_chunk((const void*)(interleaved_frames.data()), frame, frames_read);

		if (frames_read < chunk_size) break;

		frame += frames_read;
	}
}

}
}