#include <cassert>
#include <optional>
#include <format>
#include "wav_reader.h"
#include "blahdio/audio_writer.h"
#include "mackron/blahdio_dr_libs.h"

namespace blahdio {
namespace read {
namespace wav {

struct WAV
{
	WAV() = default;
	WAV(const WAV&) = delete;
	WAV(WAV&& rhs) noexcept = default;
	auto operator=(const WAV&&) -> WAV& = delete;

	auto operator=(WAV&& rhs) noexcept -> WAV&
	{
		wav_ = std::move(rhs.wav_);
		header_ = rhs.header_;
		return *this;
	}

	~WAV()
	{
		if (wav_)
		{
			drwav_uninit(wav_.get());
		}
	}

	operator bool() const { return bool(wav_); }
	operator drwav*() { return wav_.get(); }
	auto get_header_info() const { return header_; }

	[[nodiscard]] static
	auto file(std::string_view utf8_path) -> expected<WAV>
	{
		auto wav{std::make_unique<drwav>()};

		if (!dr_libs::wav::init_file(wav.get(), utf8_path))
		{
			return tl::make_unexpected(std::format("Failed to open WAV decoder for file: '{}'", utf8_path));
		}

		return WAV{std::move(wav)};
	}

	[[nodiscard]] static
	auto memory(const void* data, size_t data_size) -> expected<WAV>
	{
		auto wav{std::make_unique<drwav>()};

		if (!drwav_init_memory(wav.get(), data, data_size, nullptr))
		{
			return tl::make_unexpected("Failed to open WAV decoder for memory");
		}

		return WAV{std::move(wav)};
	}

	[[nodiscard]] static
	auto stream(drwav_read_proc on_read, drwav_seek_proc on_seek, void* user_data) -> expected<WAV>
	{
		auto wav{std::make_unique<drwav>()};

		if (!drwav_init(wav.get(), on_read, on_seek, user_data, nullptr))
		{
			return tl::make_unexpected("Failed to open WAV decoder for stream");
		}
		
		return WAV{std::move(wav)};
	}

private:

	WAV(std::unique_ptr<drwav> wav) : wav_{std::move(wav)} , header_{get_header_info(*wav_)} {}

	[[nodiscard]] static
	auto get_header_info(const drwav& wav) -> AudioDataFormat
	{
		AudioDataFormat out;

		out.frame_size = sizeof(float);
		out.num_channels = wav.channels;
		out.num_frames = wav.totalPCMFrameCount;
		out.sample_rate = wav.sampleRate;
		out.bit_depth = wav.bitsPerSample;

		return out;
	}

	std::unique_ptr<drwav> wav_;
	AudioDataFormat header_{};
};

[[nodiscard]] static
auto read_frame_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
{
	const auto read_func = [wav](float* buffer, uint32_t read_size)
	{
		return uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	return dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static
auto read_stream_data(drwav* wav, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> void
{
	const auto read_func = [wav](float* buffer, uint32_t read_size)
	{
		return uint32_t(drwav_read_pcm_frames_f32(wav, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

static
auto convert(drwav_seek_origin drwav_origin) -> AudioReader::Stream::SeekOrigin
{
	switch (drwav_origin)
	{
		case drwav_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drwav_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static
auto drwav_stream_read(void* user_data, void* buffer, size_t bytes_to_read) -> size_t
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, uint32_t(bytes_to_read));
}

static drwav_bool32 drwav_stream_seek(void* user_data, int offset, drwav_seek_origin origin)
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct WavHandler
{
	using OpenFn = std::function<expected<WAV>()>;

	WavHandler(OpenFn open_fn) : open_fn_{open_fn} {}

	auto type() const -> AudioType { return AudioType::wav; }

	auto try_read_header() -> expected<AudioDataFormat>
	{
		const auto get_header_info = [=](WAV&& wav) -> expected<AudioDataFormat>
		{
			return wav.get_header_info();
		};

		return open_fn_().and_then(get_header_info);
	}

	auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
	{
		const auto read_frames = [=](WAV&& wav)
		{
			return read_frame_data(wav, callbacks, format, chunk_size);
		};

		return open_fn_().and_then(read_frames);
	}

	[[nodiscard]]
	auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to read frames from the WAV stream (The stream is not open)");
		}

		return uint32_t(drwav_read_pcm_frames_f32(*stream_, uint64_t(frames_to_read), (float*)(buffer)));
	}

	[[nodiscard]]
	auto stream_open() -> expected<AudioDataFormat>
	{
		if (stream_)
		{
			return tl::make_unexpected("Failed to open WAV stream (It is already open)");
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
			return stream_->get_header_info();
		};

		return open_stream().and_then(get_header_info);
	}

	[[nodiscard]]
	auto stream_seek(uint64_t target_frame) -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to seek the WAV stream (The stream is not open)");
		}

		const auto result{drwav_seek_to_pcm_frame(*stream_, target_frame)};

		if (!result)
		{
			return tl::make_unexpected("Failed to seek the WAV stream for some reason");
		}

		return {};
	}

	[[nodiscard]]
	auto stream_close() -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to close WAV stream (The stream is not open)");
		}

		stream_ = std::nullopt;
		return {};
	}

private:

	OpenFn open_fn_;
	std::optional<WAV> stream_;
};

auto make_handler(std::string utf8_path) -> typed::Handler
{
	auto open_fn = [utf8_path]
	{
		return WAV::file(utf8_path);
	};

	return WavHandler{open_fn};
}

auto make_handler(const AudioReader::Stream& stream) -> typed::Handler
{
	auto open_fn = [&stream]
	{
		return WAV::stream(drwav_stream_read, drwav_stream_seek, (void*)(&stream));
	};

	return WavHandler{open_fn};
}

auto make_handler(const void* data, std::size_t data_size) -> typed::Handler
{
	auto open_fn = [data, data_size]
	{
		return WAV::memory(data, data_size);
	};

	return WavHandler{open_fn};
}

auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>
{
	std::vector<typed::Handler*> out;

	out.push_back(&handlers->wav);

#	if BLAHDIO_ENABLE_MP3
	out.push_back(&handlers->mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
	out.push_back(&handlers->flac);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
	out.push_back(&handlers->wavpack);
#	endif

	return out;
}

}}}
