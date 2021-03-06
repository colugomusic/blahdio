#pragma once

#include "read/typed_read_handler.h"

namespace blahdio {
namespace read {
namespace mp3 {

extern std::shared_ptr<typed::Handler> make_handler(const std::string& utf8_path);
extern std::shared_ptr<typed::Handler> make_handler(const AudioReader::Stream& stream);
extern std::shared_ptr<typed::Handler> make_handler(const void* data, std::size_t data_size);
extern std::vector<std::shared_ptr<typed::Handler>> make_attempt_order(const typed::Handlers& handlers);

}}}