#pragma once

#include <memory>
#include <vector>
#include "blahdio/audio_data_format.h"
#include "blahdio/audio_reader.h"
#include "blahdio/expected.h"

namespace blahdio {
namespace read {
namespace typed {

struct Handler
{
	template <
		typename T,
		typename t = std::remove_cv_t<std::remove_reference_t<T>>,
		typename e = std::enable_if_t<!std::is_same_v<Handler, t>>
	>
	Handler(T&& object) : impl_{std::make_unique<Model<T>>(std::forward<T>(object))} {}

	[[nodiscard]] auto type() const {
		return impl_->type();
	}

	[[nodiscard]] auto try_read_header() {
		return impl_->try_read_header();
	}

	[[nodiscard]] auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) {
		return impl_->read_frames(callbacks, format, chunk_size);
	}

	[[nodiscard]] auto stream_open() {
		return impl_->stream_open();
	}

	[[nodiscard]] auto stream_seek(uint64_t target_frame) {
		return impl_->stream_seek(target_frame);
	}

	[[nodiscard]] auto stream_read(void* buffer, uint32_t frames_to_read) {
		return impl_->stream_read(buffer, frames_to_read);
	}

	[[nodiscard]] auto stream_close() {
		return impl_->stream_close();
	}

private:

	struct Concept
	{
		virtual auto type() const -> AudioType = 0;
		virtual auto try_read_header() -> expected<AudioDataFormat> = 0;
		virtual auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void> = 0;
		virtual auto stream_open() -> expected<AudioDataFormat> = 0;
		virtual auto stream_seek(uint64_t target_frame) -> expected<void> = 0;
		virtual auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t> = 0;
		virtual auto stream_close() -> expected<void> = 0;
	};

	template <typename T>
	struct Model : public Concept
	{
		template <typename U>
		Model(U&& object) : object_{std::forward<U>(object)} {}

		auto type() const -> AudioType override {
			return object_.type();
		}
		auto try_read_header() -> expected<AudioDataFormat> override {
			return object_.try_read_header();
		}
		auto read_frames(AudioReader::Callbacks callbacks, const AudioDataFormat& format, uint32_t chunk_size) -> expected<void> override {
			return object_.read_frames(callbacks, format, chunk_size);
		}
		auto stream_open() -> expected<AudioDataFormat> override {
			return object_.stream_open();
		}
		auto stream_seek(uint64_t target_frame) -> expected<void> override {
			return object_.stream_seek(target_frame);
		}
		auto stream_read(void* buffer, uint32_t frames_to_read) -> expected<uint32_t> override {
			return object_.stream_read(buffer, frames_to_read);
		}
		auto stream_close() -> expected<void> override {
			return object_.stream_close();
		}

	private:

		T object_;
	};

	std::unique_ptr<Concept> impl_;
};

struct Handlers
{
#	if BLAHDIO_ENABLE_FLAC
		Handler flac;
#	endif

#	if BLAHDIO_ENABLE_MP3
		Handler mp3;
#	endif

#	if BLAHDIO_ENABLE_WAV
		Handler wav;
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		Handler wavpack;
#	endif

	auto make_type_attempt_order(AudioType type) -> std::vector<Handler*>;
};

extern auto make_handlers(std::string utf8_path) -> Handlers;
extern auto make_handlers(const AudioReader::Stream& stream) -> Handlers;
extern auto make_handlers(const void* data, std::size_t data_size) -> Handlers;

}}}
