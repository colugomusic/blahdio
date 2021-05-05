#pragma once

#include <fstream>
#include <memory>

#include "binary_reader.h"

namespace blahdio {
namespace binary {

class FileReader : public Reader
{
	std::unique_ptr<std::ifstream> file_;

	void read_chunk(std::uint32_t num_frames, char* buffer) override;

public:

	FileReader(const std::string& utf8_path, int frame_size);
};

}}