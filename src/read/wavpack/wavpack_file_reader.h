#pragma once

#include <string>
#include "wavpack_reader.h"

namespace blahdio {
namespace read {
namespace wavpack {

class FileReader : public Reader
{
	std::string utf8_path_;

	WavpackContext* open() override;

public:

	FileReader(std::string utf8_path);
};

}}}