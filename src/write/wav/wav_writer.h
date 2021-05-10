#pragma once

#include "write/typed_write_handler.h"

namespace blahdio {
namespace write {
namespace wav {

extern typed::Handler make_handler(const std::string& utf8_path, const AudioDataFormat& format);
extern typed::Handler make_handler(const AudioWriter::Stream& stream, const AudioDataFormat& format);

}}}