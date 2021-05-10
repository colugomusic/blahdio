#pragma once

#include "read/generic_reader.h"
#include "read/typed_read_handler.h"

struct WavpackContext;

namespace blahdio {
namespace read {
namespace wavpack {

class Reader : public GenericReader
{
	WavpackContext* context_ = nullptr;
	std::function<bool(float* buffer, std::uint32_t read_size)> chunk_reader_;

	virtual WavpackContext* open() = 0;

public:

	~Reader();

	bool try_read_header();
	void read_frames(Callbacks callbacks, std::uint32_t chunk_size) override;
};

extern typed::Handler make_handler(const std::string& utf8_path);
extern typed::Handler make_handler(const AudioReader::Stream& stream);
extern typed::Handler make_handler(const void* data, std::size_t data_size);

}}}