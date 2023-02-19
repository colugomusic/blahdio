#include "wavpack_reader.h"
#include "wavpack_file_reader.h"
#include "wavpack_stream_reader.h"
#include "wavpack_memory_reader.h"
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
	num_frames_ = uint64_t(WavpackGetNumSamples64(context_));
	sample_rate_ = WavpackGetSampleRate(context_);
	bit_depth_ = WavpackGetBitsPerSample(context_);

	const auto mode = WavpackGetMode(context_);

	if ((mode & MODE_FLOAT) == MODE_FLOAT)
	{
		chunk_reader_ = [this](float* buffer, uint32_t read_size)
		{
			return WavpackUnpackSamples(context_, reinterpret_cast<int32_t*>(buffer), read_size);
		};
	}
	else
	{
		chunk_reader_ = [this](float* buffer, uint32_t read_size)
		{
			const auto divisor = (1 << (bit_depth_ - 1)) - 1;

			unpacked_samples_buffer_.resize(size_t(num_channels_) * read_size);

			const auto frames_read = WavpackUnpackSamples(context_, unpacked_samples_buffer_.data(), read_size);

			unpacked_samples_buffer_.resize(frames_read * num_channels_);

			for (uint32_t i = 0; i < frames_read * num_channels_; i++)
			{
				buffer[i] = float(unpacked_samples_buffer_[i]) / divisor;
			}

			return frames_read;
		};
	}

	return true;
}

auto Reader::read_all_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void>
{
	if (!context_)
	{
		try_read_header();
	}

	if (!context_)
	{
		return tl::make_unexpected("Failed to read WavPack frames");
	}

	return do_read_all_frames(callbacks, chunk_size, chunk_reader_);
}

uint32_t Reader::read_frames(uint32_t frames_to_read, float* buffer)
{
	return chunk_reader_(buffer, frames_to_read);
}

auto Reader::do_read_all_frames(Callbacks callbacks, uint32_t chunk_size, ChunkReader chunk_reader) -> expected<void>
{
	uint64_t frame = 0;

	while (frame < num_frames_)
	{
		if (callbacks.should_abort()) break;

		std::vector<float> interleaved_frames;

		auto read_size = chunk_size;

		if (frame + read_size >= num_frames_)
		{
			read_size = uint32_t(num_frames_ - frame);
		}

		interleaved_frames.resize(size_t(read_size) * num_channels_);

		const auto frames_read = chunk_reader(interleaved_frames.data(), read_size);

		callbacks.return_chunk((const void*)(interleaved_frames.data()), frame, frames_read);

		if (frames_read < read_size)
		{
			return tl::make_unexpected("Read error");
		}

		frame += frames_read;
	}

	return {};
}

bool Reader::seek(uint64_t target_frame)
{
	return WavpackSeekSample64(context_, target_frame);
}

struct WavPackHandler
{
	using OpenFn = std::function<expected<std::shared_ptr<Reader>>()>;

	WavPackHandler(OpenFn open_fn) : open_fn_{open_fn} {}

	auto type() const -> AudioType { return AudioType::wavpack; }

	auto try_read_header() -> expected<AudioDataFormat>
	{
		const auto get_header_info = [=](std::shared_ptr<Reader> reader) -> expected<AudioDataFormat>
		{
			if (!reader->try_read_header())
			{
				return tl::make_unexpected("Failed to read WavPack header");
			}

			return reader->get_header_info();
		};

		return open_fn_().and_then(get_header_info);
	}

	auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
	{
		const auto read_frames = [=](std::shared_ptr<Reader> reader)
		{
			wavpack::Reader::Callbacks reader_callbacks;

			reader_callbacks.return_chunk = callbacks.return_chunk;
			reader_callbacks.should_abort = callbacks.should_abort;

			return reader->read_all_frames(reader_callbacks, chunk_size);
		};

		return open_fn_().and_then(read_frames);
	}

	auto stream_open() -> expected<AudioDataFormat>
	{
		if (stream_)
		{
			return tl::make_unexpected("Failed to open WavPack stream (It is already open)");
		}

		const auto open_stream = [=]() -> expected<void>
		{
			auto result{open_fn_()};

			if (!result)
			{
				return tl::make_unexpected(result.error());
			}

			stream_ = std::move(*result);
			return {};
		};

		const auto get_header_info = [=]() -> expected<AudioDataFormat>
		{
			if (!stream_->try_read_header())
			{
				return tl::make_unexpected("Failed to read WavPack header");
			}

			return stream_->get_header_info();
		};

		return open_stream().and_then(get_header_info);
	}

	auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to read frames from the WavPack stream (The stream is not open)");
		}

		return stream_->read_frames(frames_to_read, (float*)(buffer));
	}

	[[nodiscard]]
	auto stream_seek(uint64_t target_frame) -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to seek the WavPack stream (The stream is not open)");
		}

		if (!stream_->seek(target_frame))
		{
			return tl::make_unexpected("Failed to seek the WavPack stream for some reason");
		}

		return {};
	}

	[[nodiscard]]
	auto stream_close() -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to close WavPack stream (The stream is not open)");
		}

		stream_.reset();
		return {};
	}

private:

	OpenFn open_fn_;
	std::shared_ptr<Reader> stream_;
};

auto make_handler(std::string utf8_path) -> typed::Handler
{
	auto open_fn = [utf8_path = std::move(utf8_path)]
	{
		return std::make_shared<wavpack::FileReader>(utf8_path);
	};

	return WavPackHandler{open_fn};
}

auto make_handler(const AudioReader::Stream& stream) -> typed::Handler
{
	auto open_fn = [&stream]
	{
		return std::make_shared<wavpack::StreamReader>(stream);
	};

	return WavPackHandler{open_fn};
}

auto make_handler(const void* data, std::size_t data_size) -> typed::Handler
{
	auto open_fn = [data, data_size]
	{
		return std::make_shared<wavpack::MemoryReader>(data, data_size);
	};

	return WavPackHandler{open_fn};
}

auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>
{
	std::vector<typed::Handler*> out;

	out.push_back(&handlers->wavpack);

#	if BLAHDIO_ENABLE_WAV
	out.push_back(&handlers->wav);
#	endif

#	if BLAHDIO_ENABLE_MP3
	out.push_back(&handlers->mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
	out.push_back(&handlers->flac);
#	endif

	return out;
}

}}}
