# blahdio

This is an audio file reader/writer library which supports the following file types:

#### Reading
- WAV
- MP3
- FLAC
- WavPack (Pure/lossless mode only)

#### Writing
- WAV
- WavPack (Pure/lossless mode only)

You can read and write to and from files, streams or memory locations.
The read/write loops are implemented by the library. The client supplies their own callbacks to handle each chunk of frame data.
There is also an interface for reading from streams.

Blahdio is implemented using these libraries:
- https://github.com/mackron/dr_libs
- https://github.com/mackron/miniaudio
- https://github.com/dbry/WavPack
- https://github.com/nemtrif/utfcpp
- https://github.com/TartanLlama/expected
- https://github.com/fmtlib/fmt

## Examples

### Reading audio data from a file

```c++
#include <blahdio/audio_reader.h>
#include <blahdio/library_info.h>

...

// Generate a type hint from the file name. This is just looking at the
// file extension to deduce what the file's type is. The file extension
// could be wrong so this is only a hint for which file type to try first
// when reading the file header.
// 
// If the second argument is set to false, only the type deduced from the
// file extension will be tried. If it's set to true, every supported file
// type will be tried until we successfully read a valid header, but we will
// start with the type deduced from the file extension.
auto type_hint = blahdio::type_hint_for_file(utf8_file_path, true);

if (!type_hint)
{
  // Failed to deduce the type hint from the file extension.
  // Let's just fall back to WAV
  type_hint = blahdio::AudioTypeHint::wav;
}

// Create the reader
blahdio::AudioReader reader(utf8_file_path, type_hint);

// Read the header. 
const auto format = reader.read_header();

if (!format)
{
  // Failed to read the header.
  print_error_string(format.error());
  return;
}

const auto num_frames = format->num_frames;
const auto num_channels = format->num_channels;
const auto sample_rate = format->sample_rate;
const auto bit_depth = format->bit_depth;

blahdio::AudioReader::Callbacks reader_callbacks;

reader_callbacks.should_abort = []()
{
  // This callback will be entered before each
  // chunk of frame data is read.
  
  // Return true to stop reading.
  return false;
};

reader_callbacks.return_chunk = [](const void* data, std::uint64_t frame, std::uint32_t num_frames)
{
  // The library calls this for each chunk of audio data it reads.
  
  // The channel data will be interleaved.
  // A frame consists of one value for each channel.
  
  // For typed audio data (WAV, MP3, FLAC and WavPack),
  // frames will be returned as floats.
  
  // <data> points to the frame data. For typed audio data this will be
  //        (<num_frames> * (number of channels) * sizeof(float)) bytes of data
  
  // <frame> is the index of the first frame in the chunk, so if the chunk size
  //         is 512, this will be 0, 512, 1024, 1536 etc.
  
  // <num_frames> is the number of frames pointed to by <data>. This may be less than
  //              the chunk size if this is the final chunk.
};

// Read file 512 frames at a time.
// Note if we didn't already call read_header() at least once, it will
// automatically be read at this point.
const auto result = reader.read_frames(reader_callbacks, 512);

if (!result)
{
  // An error occurred.
  print_error_string(result.error());
}

```

### Writing audio data to a file

```c++
#include <blahdio/audio_writer.h>

...

blahdio::AudioDataFormat write_format;

write_format.bit_depth = 32;
write_format.num_channels = 2;
write_format.num_frames = num_frames;
write_format.sample_rate = 44100;

blahdio::AudioWriter writer("file_path.wav", blahdio::AudioType::WAV, write_format);

blahdio::AudioWriter::Callbacks writer_callbacks;

writer_callbacks.should_abort = []()
{
  // This callback will be entered before each
  // chunk of frame data is written.
  
  // Return true to stop writing.
  return false;
};

writer_callbacks.get_next_chunk = [](float* buffer, std::uint64_t frame, std::uint32_t num_frames)
{
  // <frame> is the index of the first frame to be written, so if the chunk size
  //         is 512, this will be 0, 512, 1024, 1536 etc.
  
  // <buffer> points to a buffer of
  //          (<num_frames> * (number of channels) * sizeof(float)) bytes
  
  // <num_frames> is the number of frames that should be written to <buffer>
  
  // Written channel data should be interleaved
};

// Write file 512 frames at a time
writer.write_frames(writer_callbacks, 512); // Will throw an exception if an error
                                            // occurs during writing
```
