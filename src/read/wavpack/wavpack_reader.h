#pragma once

#include "read/generic_reader.h"
#include "read/typed_read_handler.h"

struct WavpackContext;

namespace blahdio {
namespace read {
namespace wavpack {

class Reader : public GenericReader
{
public:

	~Reader();

	bool try_read_header();
	auto read_all_frames(Callbacks callbacks, uint32_t chunk_size) -> expected<void> override;
	std::uint32_t read_frames(std::uint32_t frames_to_read, float* buffer);
	bool seek(std::uint64_t target_frame);

protected:

	using ChunkReader = std::function<std::uint32_t(float* buffer, uint32_t read_size)>;

private:

	WavpackContext* context_ = nullptr;
	ChunkReader chunk_reader_;
	std::vector<std::int32_t> unpacked_samples_buffer_;

	virtual WavpackContext* open() = 0;

	virtual auto do_read_all_frames(Callbacks callbacks, uint32_t chunk_size, ChunkReader chunk_reader) -> expected<void>;

};

extern auto make_handler(std::string utf8_path) -> typed::Handler;
extern auto make_handler(const AudioReader::Stream& stream) -> typed::Handler;
extern auto make_handler(const void* data, std::size_t data_size) -> typed::Handler;
extern auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>;

}}}