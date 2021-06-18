#include "blahdio/library_info.h"

namespace blahdio {

std::vector<std::string> known_file_extensions()
{
	std::vector<std::string> out;

#	if BLAHDIO_ENABLE_FLAC
		out.push_back("FLAC");
#	endif

#	if BLAHDIO_ENABLE_MP3
		out.push_back("MP3");
#	endif

#	if BLAHDIO_ENABLE_WAV
		out.push_back("WAV");
#	endif

#	if BLAHDIO_ENABLE_WAVPACK
		out.push_back("WV");
#	endif

	return out;
}

}
