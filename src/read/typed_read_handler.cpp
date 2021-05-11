#include "typed_read_handler.h"

#include <stdexcept>

#if BLAHDIO_ENABLE_FLAC
#	include "flac/flac_reader.h"
#endif

#if BLAHDIO_ENABLE_MP3
#include "mp3/mp3_reader.h"
#endif

#if BLAHDIO_ENABLE_WAV
#include "wav/wav_reader.h"
#endif

#if BLAHDIO_ENABLE_WAVPACK
#include "wavpack/wavpack_reader.h"
#endif

namespace blahdio {
namespace read {
namespace typed {

extern Handlers make_handlers(const std::string& utf8_path);
extern Handlers make_handlers(const void* data, std::size_t data_size);

Handlers make_handlers(const std::string& utf8_path)
{
	Handlers out;

#	if BLAHDIO_ENABLE_FLAC
		out.flac = read::flac::make_handler(utf8_path);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.mp3 = read::mp3::make_handler(utf8_path);
#	endif

#	if BLAHDIO_ENABLE_WAV
		out.wav = read::wav::make_handler(utf8_path);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.wavpack = read::wavpack::make_handler(utf8_path);
#	endif

	return out;
}

Handlers make_handlers(const AudioReader::Stream& stream)
{
	Handlers out;

#	if BLAHDIO_ENABLE_FLAC
		out.flac = read::flac::make_handler(stream);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.mp3 = read::mp3::make_handler(stream);
#	endif

#	if BLAHDIO_ENABLE_WAV
		out.wav = read::wav::make_handler(stream);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.wavpack = read::wavpack::make_handler(stream);
#	endif

	return out;
}

Handlers make_handlers(const void* data, std::size_t data_size)
{
	Handlers out;

#	if BLAHDIO_ENABLE_FLAC
		out.flac = read::flac::make_handler(data, data_size);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.mp3 = read::mp3::make_handler(data, data_size);
#	endif

#	if BLAHDIO_ENABLE_WAV
		out.wav = read::wav::make_handler(data, data_size);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.wavpack = read::wavpack::make_handler(data, data_size);
#	endif

	return out;
}

static std::vector<read::typed::Handler> make_default_attempt_order(const Handlers& handlers)
{
	std::vector<typed::Handler> out;

#	if BLAHDIO_ENABLE_WAV
		out.push_back(handlers.wav);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.push_back(handlers.mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
		out.push_back(handlers.flac);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.push_back(handlers.wavpack);
#	endif

	return out;
}

std::vector<read::typed::Handler> Handlers::make_type_attempt_order(AudioType type_hint) const
{
	switch (type_hint)
	{
#		if BLAHDIO_ENABLE_FLAC
			case AudioType::FLAC: return read::flac::make_attempt_order(*this);
#		endif

#		if BLAHDIO_ENABLE_MP3
			case AudioType::MP3: return read::mp3::make_attempt_order(*this);
#		endif

#		if BLAHDIO_ENABLE_WAV
			case AudioType::WAV: return read::wav::make_attempt_order(*this);
#		endif

#		if BLAHDIO_ENABLE_WAVPACK
			case AudioType::WavPack: return read::wavpack::make_attempt_order(*this);
#		endif

		default: return make_default_attempt_order(*this);
	}
}

}}}