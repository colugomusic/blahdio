#pragma once

#include <string>
#include <tl/expected.hpp>

namespace blahdio {

template <typename T> using expected = tl::expected<T, std::string>;

} // blahdio

