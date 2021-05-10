#include "wavpack_writer.h"
#include <fstream>
#include <utf8.h>
#include <wavpack.h>

namespace blahdio {
namespace write {
namespace wavpack {

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
			write_size = std::uint32_t(format.num_frames - frame);
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
	}

	if (!WavpackFlushSamples(context))
	{
		throw std::runtime_error("Write error");
	}

	WavpackCloseFile(context);
}

typed::Handler make_handler(const std::string& utf8_path, const AudioDataFormat& format)
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

typed::Handler make_handler(const AudioWriter::Stream& stream, const AudioDataFormat& format)
{
	const auto write_func = [stream, format](AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
	{
		const auto blockout = [](void* id, void* data, int32_t bcount) -> int
		{
			auto stream = (AudioWriter::Stream*)(id);

			if (stream->write_bytes(data, bcount) != bcount) return 0;

			return 1;
		};

		wavpack_write_file(blockout, (void*)(&stream), format, callbacks, chunk_size);
	};

	return { write_func };
}

}}}