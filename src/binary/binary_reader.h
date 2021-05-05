#pragma once

#include "generic_reader.h"

namespace blahdio {
namespace binary {

class Reader : public GenericReader
{
	virtual void read_chunk(std::uint32_t num_frames, char* buffer) = 0;

public:

	void read_frames(Callbacks callbacks, std::uint32_t chunk_size) override;
};

}}