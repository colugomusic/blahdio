#pragma once

#include "read/generic_reader.h"
#include "read/typed_read_handler.h"

struct WavpackContext;

namespace blahdio {
namespace read {
namespace wavpack {

class Reader : public GenericReader, public typed::Handler
{
	WavpackContext* context_ = nullptr;
	std::function<std::uint32_t(float* buffer, std::uint32_t read_size)> chunk_reader_;

	virtual WavpackContext* open() = 0;

	virtual void do_read_frames(Callbacks callbacks, std::uint32_t chunk_size, std::function<std::uint32_t(float* buffer, std::uint32_t read_size)> chunk_reader);

public:

	~Reader();

	AudioType type() const override;

	bool try_read_header(AudioDataFormat* format) override;
	void read_frames(AudioReader::Callbacks, const AudioDataFormat& format, std::uint32_t chunk_size) override;

	bool stream_open(AudioDataFormat* format) override;
	bool stream_seek(std::uint64_t target_frame) override;
	std::uint32_t stream_read(void* buffer, std::uint32_t frames_to_read) override;
	void stream_close() override;
};

extern std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path);
extern std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream);
extern std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size);
extern std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers);

}}}