#pragma once

#include "generic_reader.h"

struct WavpackContext;

namespace blahdio {
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

}}