#include "wavpack_writer.h"
#include <fstream>
#include <vector>
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
	constexpr auto NORMALIZED_FLOAT = 127;
	constexpr auto UNNORMALIZED_FLOAT = 128;

	WavpackConfig config = { 0 };

	config.bytes_per_sample = format.bit_depth / 8;
	config.bits_per_sample = format.bit_depth;
	config.channel_mask = format.num_channels == 1 ? CFG_MONO : CFG_STEREO;
	config.num_channels = format.num_channels;
	config.sample_rate = format.sample_rate;

	switch (format.wavpack.type)
	{
		case AudioDataFormat::WavpackFormat::StorageType::Float:
		{
			config.float_norm_exp = UNNORMALIZED_FLOAT;
			break;
		}

		case AudioDataFormat::WavpackFormat::StorageType::NormalizedFloat:
		{
			config.float_norm_exp = NORMALIZED_FLOAT;
			break;
		}
	}

	if (!WavpackSetConfiguration64(context, &config, format.num_frames, nullptr))
	{
		throw std::runtime_error(WavpackGetErrorMessage(context));
	}

	if (!WavpackPackInit(context))
	{
		throw std::runtime_error(WavpackGetErrorMessage(context));
	}

	const auto int_scale = (1 << (format.bit_depth - 1)) - 1;

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

		switch (format.wavpack.type)
		{
			case AudioDataFormat::WavpackFormat::StorageType::Float:
			case AudioDataFormat::WavpackFormat::StorageType::NormalizedFloat:
			{
				if (!WavpackPackSamples(context, reinterpret_cast<std::int32_t*>(interleaved_frames.data()), write_size))
				{
					throw std::runtime_error("Write error");
				}

				break;
			}

			case AudioDataFormat::WavpackFormat::StorageType::Int:
			{
				std::vector<std::int32_t> samples(size_t(write_size) * format.num_channels);

				for (int i = 0; i < samples.size(); i++)
				{
					samples[i] = std::int32_t(double(interleaved_frames[i]) * int_scale);
				}

				if (!WavpackPackSamples(context, samples.data(), write_size))
				{
					throw std::runtime_error("Write error");
				}
				
				break;
			}
		}

		frame += write_size;
	}

	if (!WavpackFlushSamples(context))
	{
		throw std::runtime_error("Write error");
	}

	WavpackCloseFile(context);
}

static void open_file(std::ofstream* file, const std::string& utf8_path)
{
#ifdef _WIN32
	file->open((const wchar_t*)(utf8::utf8to16(utf8_path).c_str()), std::fstream::binary);
#else
	file->open(utf8_path, std::fstream::binary);
#endif
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

		open_file(&file, utf8_path);

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
