#include "wavpack_reader.h"
#include "wavpack_file_reader.h"
#include "wavpack_stream_reader.h"
#include "wavpack_memory_reader.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <wavpack.h>

namespace blahdio {
namespace read {
namespace wavpack {

Reader::~Reader()
{
	if (context_)
	{
		WavpackCloseFile(context_);
	}
}

bool Reader::try_read_header()
{
	context_ = open();

	if (!context_) return false;

	frame_size_ = sizeof(float);
	num_channels_ = WavpackGetNumChannels(context_);
	num_frames_ = std::uint64_t(WavpackGetNumSamples64(context_));
	sample_rate_ = WavpackGetSampleRate(context_);
	bit_depth_ = WavpackGetBitsPerSample(context_);

	const auto mode = WavpackGetMode(context_);

	if ((mode & MODE_FLOAT) == MODE_FLOAT)
	{
		chunk_reader_ = [this](float* buffer, std::uint32_t read_size)
		{
			return WavpackUnpackSamples(context_, reinterpret_cast<int32_t*>(buffer), read_size) == read_size;
		};
	}
	else
	{
		chunk_reader_ = [this](float* buffer, std::uint32_t read_size)
		{
			const auto divisor = (1 << (bit_depth_ - 1)) - 1;

			std::vector<std::int32_t> frames(size_t(num_channels_) * read_size);

			const auto frames_read = WavpackUnpackSamples(context_, frames.data(), read_size);

			if (frames_read < read_size) return frames_read;

			for (std::uint32_t i = 0; i < read_size * num_channels_; i++)
			{
				buffer[i] = float(frames[i]) / divisor;
			}

			return frames_read;
		};
	}

	return true;
}

void Reader::read_all_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	if (!context_) try_read_header();
	if (!context_) throw std::runtime_error("Read error");

	do_read_all_frames(callbacks, chunk_size, chunk_reader_);
}

std::uint32_t Reader::read_frames(std::uint32_t frames_to_read, float* buffer)
{
	std::vector<float> interleaved_frames(frames_to_read * num_channels_);

	return chunk_reader_(interleaved_frames.data(), frames_to_read);
}

void Reader::do_read_all_frames(Callbacks callbacks, std::uint32_t chunk_size, std::function<std::uint32_t(float* buffer, std::uint32_t read_size)> chunk_reader)
{
	std::uint64_t frame = 0;

	while (frame < num_frames_)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		auto read_size = chunk_size;

		if (frame + read_size >= num_frames_)
		{
			read_size = std::uint32_t(num_frames_ - frame);
		}

		interleaved_frames.resize(size_t(read_size) * num_channels_);

		const auto frames_read = chunk_reader(interleaved_frames.data(), read_size);

		callbacks.return_chunk((const void*)(interleaved_frames.data()), frame, frames_read);

		if (frames_read < read_size) throw std::runtime_error("Read error");

		frame += frames_read;
	}
}

bool Reader::seek(std::uint64_t target_frame)
{
	return WavpackSeekSample64(context_, target_frame);
}

struct WavPackHandler : public typed::Handler
{
	AudioType type() const override { return AudioType::WavPack; }

	virtual bool try_read_header(AudioDataFormat* format) override
	{
		auto reader = init();

		if (!reader) return false;
		if (!reader->try_read_header()) return false;

		*format = reader->get_header_info();

		return true;
	}

	void read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) override
	{
		auto reader = init();

		wavpack::Reader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader->read_all_frames(reader_callbacks, chunk_size);
	}

	bool stream_open(AudioDataFormat* format) override
	{
		if (stream_) return false;

		stream_ = init();

		if (!stream_)
		{
			return false;
		}

		*format = stream_->get_header_info();

		return true;
	}

	std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) override
	{
		if (!stream_) return 0;

		return stream_->read_frames(frames_to_read, (float*)(buffer));
	}

	bool stream_seek(std::uint64_t target_frame) override
	{
		if (!stream_) return false;

		return stream_->seek(target_frame);
	}

	void stream_close() override
	{
		stream_.reset();
	}

protected:

	std::shared_ptr<Reader> stream_;

private:

	virtual std::shared_ptr<Reader> init() = 0;
};

struct WavPackFileHandler : public WavPackHandler
{
	WavPackFileHandler(const std::string& utf8_path)
		: utf8_path_(utf8_path)
	{
	}

private:

	std::shared_ptr<Reader> init() override
	{
		return std::make_shared<wavpack::FileReader>(utf8_path_);
	}

	std::string utf8_path_;
};

struct WavPackStreamHandler : public WavPackHandler
{
	WavPackStreamHandler(const AudioReader::Stream& stream)
		: source_stream_(&stream)
	{
	}

private:

	std::shared_ptr<Reader> init() override
	{
		return std::make_shared<wavpack::StreamReader>(*source_stream_);
	}

	const AudioReader::Stream* source_stream_;
};

struct WavPackMemoryHandler : public WavPackHandler
{
	WavPackMemoryHandler(const void* data, std::size_t data_size)
		: data_(data)
		, data_size_(data_size)
	{
	}

private:

	std::shared_ptr<Reader> init() override
	{
		return std::make_shared<wavpack::MemoryReader>(data_, data_size_);
	}

	const void* data_;
	std::size_t data_size_;
};

std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path)
{
	return std::make_shared<WavPackFileHandler>(utf8_path);
}

std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream)
{
	return std::make_shared<WavPackStreamHandler>(stream);
}

std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size)
{
	return std::make_shared<WavPackMemoryHandler>(data, data_size);
}

std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers)
{
	std::vector<std::shared_ptr<typed::Handler>> out;

	out.push_back(handlers.wavpack);

#	if BLAHDIO_ENABLE_WAV
	out.push_back(handlers.wav);
#	endif

#	if BLAHDIO_ENABLE_MP3
	out.push_back(handlers.mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
	out.push_back(handlers.flac);
#	endif

	return out;
}

}}}
