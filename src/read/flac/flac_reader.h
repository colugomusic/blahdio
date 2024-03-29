#pragma once

#include "blahdio/audio_reader.h"
#include "read/typed_read_handler.h"

namespace blahdio {
namespace read {
namespace flac {

extern auto make_attempt_order(typed::Handlers* handlers) -> std::vector<typed::Handler*>;
extern auto make_handler(std::string utf8_path) -> typed::Handler;
extern auto make_handler(const AudioReader::Stream& stream) -> typed::Handler;
extern auto make_handler(const void* data, std::size_t data_size) -> typed::Handler;

}}}