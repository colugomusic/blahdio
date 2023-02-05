#pragma once

namespace blahdio {

enum class AudioType
{
	none,
	binary,
	flac,
	mp3,
	wav,
	wavpack,
};

enum class AudioTypeHint
{
	// Audio type is not deduced. It is just read
	// as raw binary data
	binary,

	// Audio type will be deduced by trying to read
	// the header as each supported type, starting
	// with the one specified
	try_flac_first,
	try_mp3_first,
	try_wav_first,
	try_wavpack_first,

	// Only the specified type will be tried.
	try_flac_only,
	try_mp3_only,
	try_wav_only,
	try_wavpack_only,
};

} // blahdio