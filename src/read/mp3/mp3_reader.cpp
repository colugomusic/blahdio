#include "mp3_reader.h"
#include "mackron/blahdio_dr_libs.h"
#include <stdexcept>

namespace blahdio {
namespace read {
namespace mp3 {

static AudioDataFormat get_header_info(drmp3* m)
{
	AudioDataFormat out;

	out.frame_size = sizeof(float);
	out.num_channels = m->channels;
	out.num_frames = drmp3_get_pcm_frame_count(m);
	out.sample_rate = m->sampleRate;
	out.bit_depth = 32;

	return out;
}

static void read_frame_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [mp3](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drmp3_read_pcm_frames_f32(mp3, read_size, buffer));
	};

	dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static void read_stream_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size)
{
	const auto read_func = [mp3](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drmp3_read_pcm_frames_f32(mp3, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

static AudioReader::Stream::SeekOrigin convert(drmp3_seek_origin drmp3_origin)
{
	switch (drmp3_origin)
	{
		case drmp3_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drmp3_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static size_t drmp3_stream_read(void* user_data, void* buffer, size_t bytes_to_read)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static drmp3_bool32 drmp3_stream_seek(void* user_data, int offset, drmp3_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct MP3Handler : public typed::Handler
{
	AudioType type() const override { return AudioType::MP3; }

	bool try_read_header(AudioDataFormat* format)
	{
		drmp3 mp3;

		if (!init(&mp3)) return false;

		*format = get_header_info(&mp3);

		drmp3_uninit(&mp3);

		return true;
	}

	void read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) override
	{
		drmp3 mp3;

		if (!init(&mp3)) throw std::runtime_error("Read error");

		read_frame_data(&mp3, callbacks, format, chunk_size);

		drmp3_uninit(&mp3);
	}

	std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) override
	{
		if (!stream_) return 0;

		return std::uint32_t(drmp3_read_pcm_frames_f32(stream_, std::uint64_t(frames_to_read), (float*)(buffer)));
	}

	bool stream_open(AudioDataFormat* format) override
	{
		if (stream_) return false;

		stream_ = new drmp3;

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

		return drmp3_seek_to_pcm_frame(stream_, target_frame);
	}

	void stream_close() override
	{
		if (!stream_) return;

		drmp3_uninit(stream_);

		delete stream_;
	}

protected:

	drmp3* stream_ = nullptr;

private:

	virtual bool init(drmp3* mp3) = 0;
};

struct MP3FileHandler : public MP3Handler
{
	MP3FileHandler(const std::string& utf8_path)
		: utf8_path_(utf8_path)
	{
	}

private:

	bool init(drmp3* mp3) override
	{
		return dr_libs::mp3::init_file(mp3, utf8_path_);
	}

	std::string utf8_path_;
};

struct MP3StreamHandler : public MP3Handler
{
	MP3StreamHandler(const AudioReader::Stream& stream)
		: source_stream_(&stream)
	{
	}

private:

	bool init(drmp3* mp3) override
	{
		return drmp3_init(mp3, drmp3_stream_read, drmp3_stream_seek, (void*)(source_stream_), nullptr);
	}

	const AudioReader::Stream* source_stream_;
};

struct MP3MemoryHandler : public MP3Handler
{
	MP3MemoryHandler(const void* data, std::size_t data_size)
		: data_(data)
		, data_size_(data_size)
	{
	}

private:

	bool init(drmp3* mp3) override
	{
		return drmp3_init_memory(mp3, data_, data_size_, nullptr);
	}

	const void* data_;
	std::size_t data_size_;
};

// File
std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path)
{
	return std::make_shared<MP3FileHandler>(utf8_path);
}

// Stream
std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream)
{
	return std::make_shared<MP3StreamHandler>(stream);
}

// Memory
std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size)
{
	return std::make_shared<MP3MemoryHandler>(data, data_size);
}

std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers)
{
	std::vector<std::shared_ptr<typed::Handler>> out;

	out.push_back(handlers.mp3);

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