#include <array>
#include <stdexcept>
#include "blahdio/audio_reader.h"
#include "blahdio/interleaving.h"
#include "dr_libs/dr_libs_utils.h"
#include "wavpack/wavpack_file_reader.h"
#include "wavpack/wavpack_memory_reader.h"
#include "binary/binary_file_reader.h"
#include "binary/binary_memory_reader.h"

namespace blahdio {

static void generic_dr_libs_frame_reader_loop(
	AudioReader::Callbacks callbacks,
	std::function<bool(float*, std::uint32_t)> read_func,
	std::uint32_t chunk_size,
	int num_channels,
	std::uint64_t num_frames)
{
	std::uint64_t frame = 0;

	while (frame < num_frames)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		auto read_size = chunk_size;

		if (frame + read_size >= num_frames)
		{
			read_size = num_frames - frame;
		}

		interleaved_frames.resize(size_t(read_size) * num_channels);

		if (!read_func(interleaved_frames.data(), read_size)) throw std::runtime_error("Read error");

		callbacks.return_chunk(frame, (const void*)(interleaved_frames.data()), read_size);

		frame += read_size;
	}
}

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

static AudioDataFormat get_header_info(const GenericReader& reader)
{
	AudioDataFormat out;

	out.frame_size = reader.get_frame_size();
	out.num_channels = reader.get_num_channels();
	out.num_frames = reader.get_num_frames();
	out.sample_rate = reader.get_sample_rate();
	out.bit_depth = reader.get_bit_depth();

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

static void read_frame_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [mp3](float* buffer, std::uint32_t read_size)
	{
		return drmp3_read_pcm_frames_f32(mp3, read_size, buffer) == read_size;
	};

	generic_dr_libs_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static void read_frame_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return drwav_read_pcm_frames_f32(wav, read_size, buffer) == read_size;
	};

	generic_dr_libs_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static AudioReader::TypeHandler make_flac_handler(const std::string& utf8_path)
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

static AudioReader::TypeHandler make_flac_handler(const void* data, std::size_t data_size)
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

static AudioReader::TypeHandler make_mp3_handler(const std::string& utf8_path)
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

static AudioReader::TypeHandler make_mp3_handler(const void* data, std::size_t data_size)
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

static AudioReader::TypeHandler make_wav_handler(const std::string& utf8_path)
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

static AudioReader::TypeHandler make_wav_handler(const void* data, std::size_t data_size)
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

static AudioReader::TypeHandler make_wavpack_handler(const std::string& utf8_path)
{
	const auto try_read_header = [utf8_path](AudioDataFormat* format) -> bool
	{
		wavpack::FileReader reader(utf8_path);

		if (!reader.try_read_header()) return false;

		*format = get_header_info(reader);

		return true;
	};

	const auto read_frames = [utf8_path](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		wavpack::FileReader reader(utf8_path);
		wavpack::Reader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_frames(reader_callbacks, chunk_size);
	};

	return { AudioType::WavPack, try_read_header, read_frames };
}

static AudioReader::TypeHandler make_wavpack_handler(const void* data, std::size_t data_size)
{
	const auto try_read_header = [data, data_size](AudioDataFormat* format) -> bool
	{
		wavpack::MemoryReader reader(data, data_size);

		if (!reader.try_read_header()) return false;

		*format = get_header_info(reader);

		return true;
	};

	const auto read_frames = [data, data_size](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		wavpack::MemoryReader reader(data, data_size);
		wavpack::Reader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_frames(reader_callbacks, chunk_size);
	};

	return { AudioType::WavPack, try_read_header, read_frames };
}

static AudioReader::BinaryHandler make_binary_handler(const std::string& utf8_path)
{
	const auto read_header = [utf8_path](int frame_size, AudioDataFormat* format)
	{
		binary::FileReader reader(utf8_path, frame_size);

		*format = get_header_info(reader);
	};

	const auto read_frames = [utf8_path](AudioReader::Callbacks callbacks, int frame_size, std::uint32_t chunk_size)
	{
		binary::FileReader reader(utf8_path, frame_size);
		binary::FileReader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_frames(reader_callbacks, chunk_size);
	};

	return { read_header, read_frames };
}

static AudioReader::BinaryHandler make_binary_handler(const void* data, std::size_t data_size)
{
	const auto read_header = [data, data_size](int frame_size, AudioDataFormat* format)
	{
		binary::MemoryReader reader(frame_size, data, data_size);

		*format = get_header_info(reader);
	};

	const auto read_frames = [data, data_size](AudioReader::Callbacks callbacks, int frame_size, std::uint32_t chunk_size)
	{
		binary::MemoryReader reader(frame_size, data, data_size);
		binary::MemoryReader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_frames(reader_callbacks, chunk_size);
	};

	return { read_header, read_frames };
}

auto AudioReader::make_type_attempt_order(AudioType type_hint) -> std::array<TypeHandler, 4>
{
	switch (type_hint)
	{
		case AudioType::FLAC: return { flac_handler_, wav_handler_, mp3_handler_, wavpack_handler_ };
		case AudioType::MP3: return { mp3_handler_, wav_handler_, flac_handler_, wavpack_handler_ };
		case AudioType::WavPack: return { wavpack_handler_, wav_handler_, mp3_handler_, flac_handler_ };
		default:
		case AudioType::WAV: return { wav_handler_, mp3_handler_, flac_handler_, wavpack_handler_ };
	}
}

void AudioReader::make_file_handlers(const std::string& utf8_path)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			make_binary_file_handler(utf8_path);
			break;
		}

		default:
		{
			make_typed_file_handlers(utf8_path);
			break;
		}
	}
}

void AudioReader::make_memory_handlers(const void* data, std::size_t data_size)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			make_binary_memory_handler(data, data_size);
			break;
		}

		default:
		{
			make_typed_memory_handlers(data, data_size);
			break;
		}
	}
}

void AudioReader::make_binary_file_handler(const std::string& utf8_path)
{
	binary_handler_ = make_binary_handler(utf8_path);
}

void AudioReader::make_binary_memory_handler(const void* data, std::size_t data_size)
{
	binary_handler_ = make_binary_handler(data, data_size);
}

void AudioReader::make_typed_file_handlers(const std::string& utf8_path)
{
	flac_handler_ = make_flac_handler(utf8_path);
	mp3_handler_ = make_mp3_handler(utf8_path);
	wav_handler_ = make_wav_handler(utf8_path);
	wavpack_handler_ = make_wavpack_handler(utf8_path);
}

void AudioReader::make_typed_memory_handlers(const void* data, std::size_t data_size)
{
	flac_handler_ = make_flac_handler(data, data_size);
	mp3_handler_ = make_mp3_handler(data, data_size);
	wav_handler_ = make_wav_handler(data, data_size);
	wavpack_handler_ = make_wavpack_handler(data, data_size);
}

void AudioReader::read_binary_header()
{
	binary_handler_.read_header(binary_frame_size_, &format_);
}

void AudioReader::read_typed_header()
{
	const auto type_handlers_to_try = make_type_attempt_order(type_hint_);

	for (auto type_handler : type_handlers_to_try)
	{
		AudioDataFormat format;

		if (type_handler && type_handler.try_read_header(&format))
		{
			active_type_handler_ = type_handler;
			format_ = format;
			return;
		}
	}

	throw std::runtime_error("File format not recognized");
}

void AudioReader::read_binary_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	binary_handler_.read_frames(callbacks, binary_frame_size_, chunk_size);
}

void AudioReader::read_typed_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	// If the header hasn't been read yet, read it now
	if (active_type_handler_.type == AudioType::None)
	{
		// Will throw if the audio type couldn't be deduced
		read_typed_header();
	}

	active_type_handler_.read_frames(callbacks, format_, chunk_size);
}

AudioReader::AudioReader(const std::string& utf8_path, AudioType type_hint)
	: type_hint_(type_hint)
{
	make_file_handlers(utf8_path);
}

AudioReader::AudioReader(const void* data, std::size_t data_size, AudioType type_hint)
	: type_hint_(type_hint)
{
	make_memory_handlers(data, data_size);
}

void AudioReader::set_binary_frame_size(std::size_t frame_size)
{
	binary_frame_size_ = frame_size;
}

void AudioReader::read_header()
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			read_binary_header();
			break;
		}

		default:
		{
			read_typed_header();
			break;
		}
	}
}

void AudioReader::read_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	switch (type_hint_)
	{
		case AudioType::Binary:
		{
			read_binary_frames(callbacks, chunk_size);
			break;
		}

		default:
		{
			read_typed_frames(callbacks, chunk_size);
			break;
		}
	}
}

}