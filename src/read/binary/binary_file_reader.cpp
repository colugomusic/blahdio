#include "binary_file_reader.h"
#include <fstream>
#include <utf8.h>

namespace blahdio {
namespace read {
namespace binary {

static std::unique_ptr<std::ifstream> open_file(const std::string& utf8_path)
{
#ifdef _WIN32
	return std::make_unique<std::ifstream>((const wchar_t*)(utf8::utf8to16(utf8_path).c_str()), std::ios::binary);
#else
	return std::make_unique<std::ifstream>(utf8_path, std::ios::binary);
#endif
}

FileReader::FileReader(const std::string& utf8_path, int frame_size)
	: file_(open_file(utf8_path))
{
	file_->seekg(std::ios::end);

	const auto file_size = file_->tellg();

	file_->seekg(std::ios::beg);

	frame_size_ = frame_size;
	num_channels_ = 1;
	num_frames_ = file_size / frame_size;
}

void FileReader::read_chunk(std::uint32_t num_frames, char* buffer)
{
	file_->read(buffer, size_t(num_frames) * frame_size_);
}

}}}