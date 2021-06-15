#include "flac_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace flac {

static AudioDataFormat get_header_info(drflac* f)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = f->channels;
	out.num_frames = f->totalPCMFrameCount;
	out.sample_rate = f->sampleRate;
	out.bit_depth = f->bitsPerSample;

	return out;
}

static void read_frame_data(drflac* flac, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [flac](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drflac_read_pcm_frames_f32(flac, read_size, buffer));
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static void read_stream_data(drflac* flac, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [flac](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drflac_read_pcm_frames_f32(flac, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

static AudioReader::Stream::SeekOrigin convert(drflac_seek_origin drflac_origin)
{
	switch (drflac_origin)
	{
		case drflac_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drflac_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static size_t drflac_stream_read(void* user_data, void* buffer, size_t bytes_to_read)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static drflac_bool32 drflac_stream_seek(void* user_data, int offset, drflac_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct FLACFileHandler : public typed::Handler
{
	FLACFileHandler(const std::string& utf8_path)
		: utf8_path_(utf8_path)
	{
	}

	AudioType type() const override { return AudioType::FLAC; }

	bool try_read_header(AudioDataFormat* format)
	{
		auto flac = dr_libs::flac::open_file(utf8_path_);

		if (!flac) return false;

		*format = get_header_info(flac);

		drflac_close(flac);

		return true;
	}

	void read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) override
	{
		auto flac = dr_libs::flac::open_file(utf8_path_);

		if (!flac) throw std::runtime_error("Read error");

		read_frame_data(flac, callbacks, format, chunk_size);

		drflac_close(flac);
	}

	bool stream_open(AudioDataFormat* format) override
	{
		if (stream_) return false;

		stream_ = dr_libs::flac::open_file(utf8_path_);

		if (!stream_) return false;

		*format = get_header_info(stream_);

		return true;
	}

	std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) override
	{
		if (!stream_) return 0;

		return std::uint32_t(drflac_read_pcm_frames_f32(stream_, std::uint64_t(frames_to_read), (float*)(buffer)));
	}

	void stream_close() override
	{
		if (!stream_) return;

		drflac_close(stream_);
	}

private:

	std::string utf8_path_;
	drflac* stream_ = nullptr;
};

// File
std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path)
{
	return std::make_shared<FLACFileHandler>(utf8_path);
	/*
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
	*/
}

// Stream
std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream)
{
	return nullptr;
	/*
	const auto try_read_header = [stream](AudioDataFormat* format) -> bool
	{
		auto flac = drflac_open(drflac_stream_read, drflac_stream_seek, (void*)(&stream), nullptr);

		if (!flac) return false;

		*format = get_header_info(flac);

		drflac_close(flac);

		return true;
	};

	const auto read_frames = [stream](AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
	{
		auto flac = drflac_open(drflac_stream_read, drflac_stream_seek, (void*)(&stream), nullptr);

		if (!flac) throw std::runtime_error("Read error");

		read_stream_data(flac, callbacks, format, chunk_size);

		drflac_close(flac);
	};

	return { AudioType::FLAC, try_read_header, read_frames };
	*/
}

// Memory
std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size)
{
	return nullptr;
	/*
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
	*/
}

std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers)
{
	std::vector<std::shared_ptr<typed::Handler>> out;

	out.push_back(handlers.flac);

#	if BLAHDIO_ENABLE_WAV
		out.push_back(handlers.wav);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.push_back(handlers.mp3);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.push_back(handlers.wavpack);
#	endif

	return out;
}

}}}