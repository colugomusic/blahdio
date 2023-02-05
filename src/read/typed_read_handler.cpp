#include <stdexcept>
#include "typed_read_handler.h"

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

auto make_handlers(std::string utf8_path) -> Handlers
{
	return
	{
#	if BLAHDIO_ENABLE_FLAC
		read::flac::make_handler(utf8_path),
#	endif

#	if BLAHDIO_ENABLE_MP3
		read::mp3::make_handler(utf8_path),
#	endif

#	if BLAHDIO_ENABLE_WAV
		read::wav::make_handler(utf8_path),
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		read::wavpack::make_handler(utf8_path)
#	endif
	};
}

auto make_handlers(const AudioReader::Stream& stream) -> Handlers
{
	return
	{
#	if BLAHDIO_ENABLE_FLAC
		read::flac::make_handler(stream),
#	endif

#	if BLAHDIO_ENABLE_MP3
		read::mp3::make_handler(stream),
#	endif

#	if BLAHDIO_ENABLE_WAV
		read::wav::make_handler(stream),
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		read::wavpack::make_handler(stream)
#	endif
	};
}

auto make_handlers(const void* data, size_t data_size) -> Handlers
{
	return
	{
#	if BLAHDIO_ENABLE_FLAC
		read::flac::make_handler(data, data_size),
#	endif

#	if BLAHDIO_ENABLE_MP3
		read::mp3::make_handler(data, data_size),
#	endif

#	if BLAHDIO_ENABLE_WAV
		read::wav::make_handler(data, data_size),
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		read::wavpack::make_handler(data, data_size)
#	endif
	};
}

static auto make_default_attempt_order(Handlers* handlers) -> std::vector<typed::Handler*>
{
	std::vector<typed::Handler*> out;

#	if BLAHDIO_ENABLE_WAV
		out.push_back(&handlers->wav);
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.push_back(&handlers->mp3);
#	endif

#	if BLAHDIO_ENABLE_FLAC
		out.push_back(&handlers->flac);
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.push_back(&handlers->wavpack);
#	endif

	return out;
}

auto Handlers::make_type_attempt_order(AudioType type_hint) -> std::vector<typed::Handler*>
{
	switch (type_hint)
	{
#		if BLAHDIO_ENABLE_FLAC
			case AudioType::FLAC: return read::flac::make_attempt_order(this);
#		endif

#		if BLAHDIO_ENABLE_MP3
			case AudioType::MP3: return read::mp3::make_attempt_order(this);
#		endif

#		if BLAHDIO_ENABLE_WAV
			case AudioType::WAV: return read::wav::make_attempt_order(this);
#		endif

#		if BLAHDIO_ENABLE_WAVPACK
			case AudioType::WavPack: return read::wavpack::make_attempt_order(this);
#		endif

		default: return make_default_attempt_order(this);
	}
}

}}}