# blahdio

This is an audio file reader/writer library which supports the following file types:

## Reading
- WAV
- MP3
- FLAC
- WavPack
- Untyped data

## Writing
- WAV
- WavPack

You can read and write to and from files or memory locations using the same interface.
The read/write loops are implemented by the library. The user supplies their own callbacks to handle each chunk of frame data.

Blahdio is implemented using these libraries:
- https://github.com/mackron/miniaudio
- https://github.com/dbry/WavPack
- https://github.com/nemtrif/utfcpp

## Examples

### Reading audio data from a file

```c++
// Assume that the file type is WAV. Blahdio will try that first, but if it fails
// to read a valid WAV header it will also try MP3, FLAC and WavPack.
blahdio::AudioReader reader(utf8_file_path, blahdio::AudioType::WAV);

// Read the header. 
reader.read_header(); // Will throw an exception if the file
                      // type couldn't be deduced.

const auto num_frames = reader.get_num_frames();
const auto num_channels = reader.get_num_channels();
const auto sample_rate = reader.get_sample_rate();
const auto bit_depth = reader.get_bit_depth();

blahdio::AudioReader::Callbacks reader_callbacks;

reader_callbacks.should_abort = []()
{
  // This callback will be entered before each
  // chunk of frame data is read.
  
  // Return true to stop reading.
  return false;
};

reader_callbacks.return_chunk = [](std::uint64_t frame, const void* data, std::uint32_t size)
{
  // The library calls this for each chunk of audio data it reads.
  
  // The returned frame data will be interleaved.
  
  // For typed audio data (WAV, MP3, FLAC and WavPack), frames will be returned
  // as 32-bit floats.
  
  // <frame> is the index of the first frame in the chunk, so if the chunk size
  //         is 512, this will be 0, 512, 1024, 1536 etc.
  
  // <data> points to the frame data. For typed audio data this will be
  //        (<size> * (number of channels) * sizeof(float)) bytes of data
  
  // <size> is the number of frames pointed to by <data>. This may be less than
  //        the chunk size if this is the final chunk.
};

// Read file 512 frames at a time
reader.read_frames(reader_callbacks, 512); // May throw an exception if an error
                                           // occurs during reading
```
