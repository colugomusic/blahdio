#include "binary_reader.h"
#include "binary_file_reader.h"
#include "binary_stream_reader.h"
#include "binary_memory_reader.h"
#include <vector>

namespace blahdio {
namespace read {
namespace binary {

void Reader::read_all_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	std::uint64_t frame = 0;

	while (frame < num_frames_)
	{
		if (callbacks.should_abort()) break;

		auto frames_to_read = chunk_size;

		if (frame + frames_to_read >= num_frames_)
		{
			frames_to_read = std::uint32_t(num_frames_ - frame);
		}

		std::vector<char> frame_data(size_t(frames_to_read) * frame_size_);

		read_chunk(frames_to_read, frame_data.data());

		callbacks.return_chunk((const void*)(frame_data.data()), frame, frames_to_read);

		frame += frames_to_read;
	}
}

Handler make_handler(const std::string& utf8_path)
{
	const auto read_header = [utf8_path](int frame_size, AudioDataFormat* format)
	{
		binary::FileReader reader(utf8_path, frame_size);

		*format = reader.get_header_info();
	};

	const auto read_frames = [utf8_path](blahdio::AudioReader::Callbacks callbacks, int frame_size, std::uint32_t chunk_size)
	{
		binary::FileReader reader(utf8_path, frame_size);
		binary::FileReader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_all_frames(reader_callbacks, chunk_size);
	};

	return { read_header, read_frames };
}

Handler make_handler(const AudioReader::Stream& stream)
{
	const auto read_header = [stream](int frame_size, AudioDataFormat* format)
	{
		binary::StreamReader reader(stream, frame_size);

		*format = reader.get_header_info();
	};

	const auto read_frames = [stream](blahdio::AudioReader::Callbacks callbacks, int frame_size, std::uint32_t chunk_size)
	{
		binary::StreamReader reader(stream, frame_size);
		binary::StreamReader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_all_frames(reader_callbacks, chunk_size);
	};

	return { read_header, read_frames };
}

Handler make_handler(const void* data, std::size_t data_size)
{
	const auto read_header = [data, data_size](int frame_size, AudioDataFormat* format)
	{
		binary::MemoryReader reader(frame_size, data, data_size);

		*format = reader.get_header_info();
	};

	const auto read_frames = [data, data_size](blahdio::AudioReader::Callbacks callbacks, int frame_size, std::uint32_t chunk_size)
	{
		binary::MemoryReader reader(frame_size, data, data_size);
		binary::MemoryReader::Callbacks reader_callbacks;

		reader_callbacks.return_chunk = callbacks.return_chunk;
		reader_callbacks.should_abort = callbacks.should_abort;

		reader.read_all_frames(reader_callbacks, chunk_size);
	};

	return { read_header, read_frames };
}

}}}