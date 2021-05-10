#include "typed_read_handler.h"
#include "flac/flac_reader.h"
#include "mp3/mp3_reader.h"
#include "wav/wav_reader.h"
#include "wavpack/wavpack_reader.h"

namespace blahdio {
namespace read {
namespace typed {

extern Handlers make_handlers(const std::string& utf8_path);
extern Handlers make_handlers(const void* data, std::size_t data_size);

Handlers make_handlers(const std::string& utf8_path)
{
	Handlers out;

	out.flac = read::flac::make_handler(utf8_path);
	out.mp3 = read::mp3::make_handler(utf8_path);
	out.wav = read::wav::make_handler(utf8_path);
	out.wavpack = read::wavpack::make_handler(utf8_path);

	return out;
}

Handlers make_handlers(const AudioReader::Stream& stream)
{
	Handlers out;

	out.flac = read::flac::make_handler(stream);
	out.mp3 = read::mp3::make_handler(stream);
	out.wav = read::wav::make_handler(stream);
	out.wavpack = read::wavpack::make_handler(stream);

	return out;
}

Handlers make_handlers(const void* data, std::size_t data_size)
{
	Handlers out;

	out.flac = read::flac::make_handler(data, data_size);
	out.mp3 = read::mp3::make_handler(data, data_size);
	out.wav = read::wav::make_handler(data, data_size);
	out.wavpack = read::wavpack::make_handler(data, data_size);

	return out;
}

std::array<read::typed::Handler, 4> Handlers::make_type_attempt_order(AudioType type_hint) const
{
	switch (type_hint)
	{
		case AudioType::FLAC: return { flac, wav, mp3, wavpack };
		case AudioType::MP3: return { mp3, wav, flac, wavpack };
		case AudioType::WavPack: return { wavpack, wav, mp3, flac };
		default:
		case AudioType::WAV: return { wav, mp3, flac, wavpack };
	}
}

}}}