#include <cassert>
#include <optional>
#include <fmt/format.h>
#include "mp3_reader.h"
#include "mackron/blahdio_dr_libs.h"

namespace blahdio {
namespace read {
namespace mp3 {

struct MP3
{
	MP3() = delete;
	MP3(const MP3&) = delete;
	auto operator=(const MP3&&) -> MP3& = delete;

	MP3(MP3&& rhs) : mp3_{rhs.mp3_}, header_{rhs.header_} { rhs.mp3_ = std::nullopt; }
	auto operator=(MP3&& rhs) -> MP3& { mp3_ = rhs.mp3_; rhs.mp3_ = std::nullopt; return *this; }

	~MP3()
	{
		if (mp3_)
		{
			drmp3_uninit(&mp3_.value());
		}
	}

	operator bool() const { return mp3_.has_value(); }
	operator drmp3*() { return &mp3_.value(); }
	auto get_header_info() const { return header_; }

	[[nodiscard]] static
	auto file(std::string_view utf8_path) -> expected<MP3>
	{
		drmp3 mp3;

		if (!dr_libs::mp3::init_file(&mp3, utf8_path))
		{
			return tl::make_unexpected(fmt::format("Failed to open MP3 decoder for file: '{}'", utf8_path));
		}

		return MP3{mp3};
	}

	[[nodiscard]] static
	auto memory(const void* data, size_t data_size) -> expected<MP3>
	{
		drmp3 mp3;

		if (!drmp3_init_memory(&mp3, data, data_size, nullptr))
		{
			return tl::make_unexpected("Failed to open MP3 decoder for memory");
		}

		return MP3{mp3};
	}

	[[nodiscard]] static
	auto stream(drmp3_read_proc on_read, drmp3_seek_proc on_seek, void* user_data) -> expected<MP3>
	{
		drmp3 mp3;

		if (!drmp3_init(&mp3, on_read, on_seek, user_data, nullptr))
		{
			return tl::make_unexpected("Failed to open MP3 decoder for stream");
		}
		
		return MP3{mp3};
	}

private:

	MP3(drmp3 mp3) : mp3_{mp3}, header_{get_header_info(&mp3)} {}

	[[nodiscard]] static
	auto get_header_info(drmp3* mp3) -> AudioDataFormat
	{
		assert (mp3);

		AudioDataFormat out;

		out.frame_size = sizeof(float);
		out.num_channels = mp3->channels;
		out.num_frames = drmp3_get_pcm_frame_count(mp3);
		out.sample_rate = mp3->sampleRate;
		out.bit_depth = 32;

		return out;
	}

	std::optional<drmp3> mp3_{};
	AudioDataFormat header_{};
};

static
auto read_frame_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
{
	const auto read_func = [mp3](float* buffer, uint32_t read_size)
	{
		return uint32_t(drmp3_read_pcm_frames_f32(mp3, read_size, buffer));
	};

	return dr_libs::generic_frame_reader_loop(callbacks, read_func, chunk_size, format.num_channels, format.num_frames);
}

static
auto read_stream_data(drmp3* mp3, AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> void
{
	const auto read_func = [mp3](float* buffer, uint32_t read_size)
	{
		return uint32_t(drmp3_read_pcm_frames_f32(mp3, read_size, buffer));
	};

	dr_libs::generic_stream_reader_loop(callbacks, read_func, chunk_size, format.num_channels);
}

[[nodiscard]] static
auto convert(drmp3_seek_origin drmp3_origin) -> AudioReader::Stream::SeekOrigin
{
	switch (drmp3_origin)
	{
		case drmp3_seek_origin_start: return AudioReader::Stream::SeekOrigin::Start;
		case drmp3_seek_origin_current: default: return AudioReader::Stream::SeekOrigin::Current;
	}
}

static
auto drmp3_stream_read(void* user_data, void* buffer, size_t bytes_to_read) -> size_t
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->read_bytes(buffer, uint32_t(bytes_to_read));
}

static
auto drmp3_stream_seek(void* user_data, int offset, drmp3_seek_origin origin) -> drmp3_bool32 
{
	const auto stream = (AudioReader::Stream*)(user_data);

	return stream->seek(convert(origin), offset);
}

struct MP3Handler
{
	using OpenFn = std::function<expected<MP3>()>;

	MP3Handler(OpenFn open_fn) : open_fn_{open_fn} {}

	auto type() const -> AudioType { return AudioType::MP3; }

	[[nodiscard]]
	auto try_read_header() -> expected<AudioDataFormat>
	{
		const auto get_header_info = [=](MP3&& mp3) -> expected<AudioDataFormat>
		{
			return mp3.get_header_info();
		};

		return open_fn_().and_then(get_header_info);
	}

	[[nodiscard]]
	auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void>
	{
		const auto read_frames = [=](MP3&& mp3)
		{
			return read_frame_data(mp3, callbacks, format, chunk_size);
		};

		return open_fn_().and_then(read_frames);
	}

	[[nodiscard]]
	auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to read frames from the MP3 stream (The stream is not open)");
		}

		return uint32_t(drmp3_read_pcm_frames_f32(*stream_, uint64_t(frames_to_read), (float*)(buffer)));
	}

	[[nodiscard]]
	auto stream_open() -> expected<AudioDataFormat>
	{
		if (stream_)
		{
			return tl::make_unexpected("Failed to open MP3 stream (It is already open)");
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
			return tl::make_unexpected("Failed to seek the MP3 stream (The stream is not open)");
		}

		const auto result{drmp3_seek_to_pcm_frame(*stream_, target_frame)};

		if (!result)
		{
			return tl::make_unexpected("Failed to seek the MP3 stream for some reason");
		}

		return {};
	}

	[[nodiscard]]
	auto stream_close() -> expected<void>
	{
		if (!stream_)
		{
			return tl::make_unexpected("Failed to close MP3 stream (The stream is not open)");
		}

		stream_ = std::nullopt;
		return {};
	}

private:

	OpenFn open_fn_;
	std::optional<MP3> stream_;
};

auto make_handler(std::string utf8_path) -> typed::Handler
{
	auto open_fn = [utf8_path]
	{
		return MP3::file(utf8_path);
	};

	return MP3Handler{open_fn};
}

auto make_handler(const AudioReader::Stream& stream) -> typed::Handler
{
	auto open_fn = [&stream]
	{
		return MP3::stream(drmp3_stream_read, drmp3_stream_seek, (void*)(&stream));
	};

	return MP3Handler{open_fn};
}

auto make_handler(const void* data, std::size_t data_size) -> typed::Handler
{
	auto open_fn = [data, data_size]
	{
		return MP3::memory(data, data_size);
	};

	return MP3Handler{open_fn};
}

auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>
{
	std::vector<typed::Handler*> out;

	out.push_back(&handlers->mp3);

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
