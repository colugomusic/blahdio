#include "wavpack_file_reader.h"
#include <cassert>
#include <wavpack.h>

namespace blahdio {
namespace read {
namespace wavpack {

FileReader::FileReader(std::string utf8_path)
	: utf8_path_(std::move(utf8_path))
{
}

WavpackContext* FileReader::open()
{
	assert (!utf8_path_.empty());

	int flags = 0;

	flags |= OPEN_2CH_MAX;
	//flags |= OPEN_NORMALIZE;

#ifdef _WIN32
	flags |= OPEN_FILE_UTF8;
#endif

	char error[80];

	return WavpackOpenFileInput(utf8_path_.c_str(), error, flags, 0);
}

}}}