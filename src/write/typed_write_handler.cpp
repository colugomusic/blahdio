#include "typed_write_handler.h"

#include <stdexcept>

#if BLAHDIO_ENABLE_WAV
#	include "write/wav/wav_writer.h"
#endif

#if BLAHDIO_ENABLE_WAVPACK
#	include "write/wavpack/wavpack_writer.h"
#endif

namespace blahdio {
namespace write {
namespace typed {

Handler make_handler(const AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
#		if BLAHDIO_ENABLE_WAV
			case AudioType::wav: return write::wav::make_handler(stream, format);
#		endif

#		if BLAHDIO_ENABLE_WAVPACK
			case AudioType::wavpack: return write::wavpack::make_handler(stream, format);
#		endif

		default: throw std::runtime_error("Couldn't find writer");
	}
}

Handler make_handler(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
#		if BLAHDIO_ENABLE_WAV
			case AudioType::wav: return write::wav::make_handler(utf8_path, format);
#		endif

#		if BLAHDIO_ENABLE_WAVPACK
			case AudioType::wavpack: return write::wavpack::make_handler(utf8_path, format);
#		endif

		default: throw std::runtime_error("Couldn't find writer");
	}
}
}}}