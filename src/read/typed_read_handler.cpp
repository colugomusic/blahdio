#include <cassert>
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

auto Handlers::make_type_attempt_order(AudioTypeHint type_hint) -> std::vector<typed::Handler*>
{
	switch (type_hint)
	{
#		if BLAHDIO_ENABLE_FLAC
			case AudioTypeHint::try_flac_first: return read::flac::make_attempt_order(this);
			case AudioTypeHint::try_flac_only: return { &flac };
#		endif

#		if BLAHDIO_ENABLE_MP3
			case AudioTypeHint::try_mp3_first: return read::mp3::make_attempt_order(this);
			case AudioTypeHint::try_mp3_only: return { &mp3 };
#		endif

#		if BLAHDIO_ENABLE_WAV
			case AudioTypeHint::try_wav_first: return read::wav::make_attempt_order(this);
			case AudioTypeHint::try_wav_only: return { &wav };
#		endif

#		if BLAHDIO_ENABLE_WAVPACK
			case AudioTypeHint::try_wavpack_first: return read::wavpack::make_attempt_order(this);
			case AudioTypeHint::try_wavpack_only: return { &wavpack };
#		endif

		default:
		{
			assert (false);
			return {};
		}
	}
}

}}}