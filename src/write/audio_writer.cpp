#include "blahdio/audio_writer.h"
#include "write/wav/wav_writer.h"
#include "write/wavpack/wavpack_writer.h"

namespace blahdio {

namespace impl {

class AudioWriter
{
public:

	AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format);
	AudioWriter(const blahdio::AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format);

	void write_frames(blahdio::AudioWriter::Callbacks callbacks, std::uint32_t chunk_size);

private:

	write::typed::Handler typed_handler_;
};

void AudioWriter::write_frames(blahdio::AudioWriter::Callbacks callbacks, std::uint32_t chunk_size)
{
	typed_handler_.write_frames(callbacks, chunk_size);
}

static write::typed::Handler make_type_handler(const blahdio::AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
		case AudioType::WavPack: return write::wavpack::make_handler(stream, format);
		case AudioType::WAV: default: return write::wav::make_handler(stream, format);
	}
}

static write::typed::Handler make_type_handler(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
{
	switch (type)
	{
		case AudioType::WavPack: return write::wavpack::make_handler(utf8_path, format);
		case AudioType::WAV: default: return write::wav::make_handler(utf8_path, format);
	}
}

AudioWriter::AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
	: typed_handler_(make_type_handler(utf8_path, type, format))
{
}

AudioWriter::AudioWriter(const blahdio::AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format)
	: typed_handler_(make_type_handler(stream, type, format))
{
}

} // impl

AudioWriter::AudioWriter(const std::string& utf8_path, AudioType type, const AudioDataFormat& format)
	: impl_(new impl::AudioWriter(utf8_path, type, format))
{
}

AudioWriter::AudioWriter(const blahdio::AudioWriter::Stream& stream, AudioType type, const AudioDataFormat& format)
	: impl_(new impl::AudioWriter(stream, type, format))
{
}

AudioWriter::~AudioWriter()
{
	delete impl_;
}

void AudioWriter::write_frames(Callbacks callbacks, std::uint32_t chunk_size)
{
	impl_->write_frames(callbacks, chunk_size);
}


}
