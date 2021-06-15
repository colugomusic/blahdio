#include "wav_reader.h"
#include "blahdio/audio_writer.h"
#include "mackron/blahdio_dr_libs.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace wav {

static AudioDataFormat get_header_info(drwav* w)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = w->channels;
	out.num_frames = w->totalPCMFrameCount;
	out.sample_rate = w->sampleRate;
	out.bit_depth = w->bitsPerSample;

	return out;
}

static void read_frame_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static void read_stream_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [wav](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

static AudioReader::Stream::SeekOrigin convert(drwav_seek_origin drwav_origin)
{
	switch (drwav_origin)
	{
		case drwav_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drwav_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static size_t drwav_stream_read(void* user_data, void* buffer, size_t bytes_to_read)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static drwav_bool32 drwav_stream_seek(void* user_data, int offset, drwav_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct WavHandler : public typed::Handler
{
	AudioType type() const override { return AudioType::WAV; }

	bool try_read_header(AudioDataFormat* format)
	{
		drwav wav;

		if (!init(&wav)) return false;

		*format = get_header_info(&wav);

		drwav_uninit(&wav);

		return true;
	}

	void read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) override
	{
		drwav wav;

		if (!init(&wav)) throw std::runtime_error("Read error");

		read_frame_data(&wav, callbacks, format, chunk_size);

		drwav_uninit(&wav);
	}

	std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) override
	{
		if (!stream_) return 0;

		return std::uint32_t(drwav_read_pcm_frames_f32(stream_, std::uint64_t(frames_to_read), (float*)(buffer)));
	}

	bool stream_open(AudioDataFormat* format) override
	{
		if (stream_) return false;

		stream_ = new drwav;

		if (!init(stream_))
		{
			delete stream_;
			return false;
		}

		*format = get_header_info(stream_);

		return true;
	}

	bool stream_seek(std::uint64_t target_frame) override
	{
		if (!stream_) return false;

		return drwav_seek_to_pcm_frame(stream_, target_frame);
	}

	void stream_close() override
	{
		if (!stream_) return;

		drwav_uninit(stream_);

		delete stream_;
	}

protected:

	drwav* stream_ = nullptr;

private:

	virtual bool init(drwav* wav) = 0;
};

struct WavFileHandler : public WavHandler
{
	WavFileHandler(const std::string& utf8_path)
		: utf8_path_(utf8_path)
	{
	}

private:

	bool init(drwav* wav) override
	{
		return dr_libs::wav::init_file(wav, utf8_path_);
	}

	std::string utf8_path_;
};

struct WavStreamHandler : public WavHandler
{
	WavStreamHandler(const AudioReader::Stream& stream)
		: source_stream_(&stream)
	{
	}

private:

	bool init(drwav* wav) override
	{
		return drwav_init(wav, drwav_stream_read, drwav_stream_seek, (void*)(source_stream_), nullptr);
	}

	const AudioReader::Stream* source_stream_;
};

struct WavMemoryHandler : public WavHandler
{
	WavMemoryHandler(const void* data, std::size_t data_size)
		: data_(data)
		, data_size_(data_size)
	{
	}

private:

	bool init(drwav* wav) override
	{
		return drwav_init_memory(wav, data_, data_size_, nullptr);
	}

	const void* data_;
	std::size_t data_size_;
};

// File
std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path)
{
	return std::make_shared<WavFileHandler>(utf8_path);
}

// Stream
std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream)
{
	return std::make_shared<WavStreamHandler>(stream);
}

// Memory
std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size)
{
	return std::make_shared<WavMemoryHandler>(data, data_size);
}

std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers)
{
	std::vector<std::shared_ptr<typed::Handler>> out;

	out.push_back(handlers.wav);

#	if BLAHDIO_ENABLE_MP3
	out.push_back(handlers.mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
	out.push_back(handlers.flac);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
	out.push_back(handlers.wavpack);
#	endif

	return out;
}

}}}
