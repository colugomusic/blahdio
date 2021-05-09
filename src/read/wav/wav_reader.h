#pragma once

#include "read/typed_read_handler.h"

namespace blahdio {
namespace read {
namespace wav {

extern typed::Handler make_handler(const std::string& utf8_path);
extern typed::Handler make_handler(const void* data, std::size_t data_size);

}}}