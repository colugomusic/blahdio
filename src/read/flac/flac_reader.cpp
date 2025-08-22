#include <cassert>
#include <optional>
#include <format>
#include "flac_reader.h"
#include "mackron/blahdio_dr_libs.h"

namespace blahdio {
namespace read {
namespace flac {

struct FLAC
{
	FLAC() = delete;
	FLAC(const FLAC&) = delete;
	auto operator=(const FLAC&&) -> FLAC& = delete;

	FLAC(FLAC&& rhs) : flac_{rhs.flac_}, header_{rhs.header_} { rhs.flac_ = nullptr; }
	auto operator=(FLAC&& rhs) -> FLAC& { flac_ = rhs.flac_; rhs.flac_ = nullptr; return *this; }

	~FLAC()
	{
		if (flac_)
		{
			drflac_close(flac_);
		}
	}

	operator bool() const { return flac_; }
	operator drflac*() const { return flac_; }
	auto get_header_info() const { return header_; }

	[[nodiscard]] static
	auto file(std::string_view utf8_path) -> expected<FLAC>
	{
		const auto flac{dr_libs::flac::open_file(utf8_path)};

		if (!flac)
		{
			return tl::make_unexpected(std::format("Failed to open FLAC decoder for file: '{}'", utf8_path));
		}

		return FLAC{flac};
	}

	[[nodiscard]] static
	auto memory(const void* data, size_t data_size) -> expected<FLAC>
	{
		const auto flac{drflac_open_memory(data, data_size, nullptr)};

		if (!flac)
		{
			return tl::make_unexpected("Failed to open FLAC decoder for memory");
		}

		return FLAC{flac};
	}

	[[nodiscard]] static
	auto stream(drflac_read_proc on_read, drflac_seek_proc on_seek, void* user_data) -> expected<FLAC>
	{
		const auto flac{drflac_open(on_read, on_seek, nullptr, user_data, nullptr)};

		if (!flac)
		{
			return tl::make_unexpected("Failed to open FLAC decoder for stream");
		}
		
		return FLAC{flac};
	}

private:

	FLAC(drflac* flac) : flac_{flac} , header_{get_header_info(flac)} {}

	[[nodiscard]] static
	auto get_header_info(drflac* flac) -> AudioDataFormat
	{
		assert (flac);

		AudioDataFormat out;

		out.frame_size = sizeof(float);
		out.num_channels = flac->channels;
		out.num_frames = flac->totalPCMFrameCount;
		out.sample_rate = flac->sampleRate;
		out.bit_depth = flac->bitsPerSample;

		return out;
	}


	drflac* flac_{};
	AudioDataFormat header_{};
};

[[nodiscard]] static
auto read_frame_data(drflac* flac, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
{
	const auto read_func = [flac](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drflac_read_pcm_frames_f32(flac, read_size, buffer));
	};

	return dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static
auto read_stream_data(drflac* flac, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> void
{
	const auto read_func = [flac](float* buffer, std::uint32_t read_size)
	{
		return std::uint32_t(drflac_read_pcm_frames_f32(flac, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

static
auto convert(drflac_seek_origin drflac_origin) -> AudioReader::Stream::SeekOrigin
{
	switch (drflac_origin)
	{
		case DRFLAC_SEEK_SET: return AudioReader::Stream::SeekOrigin::Start;
		case DRFLAC_SEEK_CUR: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static
auto drflac_stream_read(void* user_data, void* buffer, size_t bytes_to_read) -> size_t
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, std::uint32_t(bytes_to_read));
}

static
auto drflac_stream_seek(void* user_data, int offset, drflac_seek_origin origin) -> drflac_bool32
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct FLACHandler
{
	using OpenFn = std::function<expected<FLAC>()>;

	FLACHandler(OpenFn open_fn) : open_fn_{open_fn} {}

	auto type() const -> AudioType { return AudioType::flac; }

	[[nodiscard]]
	auto try_read_header() -> expected<AudioDataFormat>
	{
		const auto get_header_info = [=](FLAC&& flac) -> expected<AudioDataFormat>
		{
			return flac.get_header_info();
		};

		return open_fn_().and_then(get_header_info);
	}

	[[nodiscard]]
	auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
	{
		const auto read_frames = [=](FLAC&& flac)
		{
			return read_frame_data(flac, callbacks, format, chunk_size);
		};

		return open_fn_().and_then(read_frames);
	}

	[[nodiscard]]
	auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to read frames from the FLAC stream (The stream is not open)");
		}

		return uint32_t(drflac_read_pcm_frames_f32(*stream_, uint64_t(frames_to_read), (float*)(buffer)));
	}

	[[nodiscard]]
	auto stream_open() -> expected<AudioDataFormat>
	{
		if (stream_)
		{
			return tl::make_unexpected("Failed to open FLAC stream (It is already open)");
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
			return tl::make_unexpected("Failed to seek the FLAC stream (The stream is not open)");
		}

		const auto result{drflac_seek_to_pcm_frame(*stream_, target_frame)};

		if (!result)
		{
			return tl::make_unexpected("Failed to seek the FLAC stream for some reason");
		}

		return {};
	}

	[[nodiscard]]
	auto stream_close() -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to close FLAC stream (The stream is not open)");
		}

		stream_ = std::nullopt;
		return {};
	}

private:

	OpenFn open_fn_;
	std::optional<FLAC> stream_;
};

auto make_handler(std::string utf8_path) -> typed::Handler
{
	auto open_fn = [utf8_path]
	{
		return FLAC::file(utf8_path);
	};

	return FLACHandler{open_fn};
}

auto make_handler(const AudioReader::Stream& stream) -> typed::Handler
{
	auto open_fn = [&stream]
	{
		return FLAC::stream(drflac_stream_read, drflac_stream_seek, (void*)(&stream));
	};

	return FLACHandler{open_fn};
}

auto make_handler(const void* data, std::size_t data_size) -> typed::Handler
{
	auto open_fn = [data, data_size]
	{
		return FLAC::memory(data, data_size);
	};

	return FLACHandler{open_fn};
}

auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>
{
	std::vector<typed::Handler*> out;

	out.push_back(&handlers->flac);

#	if BLAHDIO_ENABLE_WAV
		out.push_back(&handlers->flac);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.push_back(&handlers->mp3);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.push_back(&handlers->wavpack);
#	endif

	return out;
}

}}}
