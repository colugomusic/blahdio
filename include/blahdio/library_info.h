#pragma once

#include <array>
#include <string>
#include <vector>
#include "blahdio/expected.h"
#include "blahdio/audio_type.h"

namespace blahdio {

[[nodiscard]] extern
auto get_file_extension(AudioType type) -> std::string_view;

[[nodiscard]] extern
auto known_file_extensions() -> std::vector<std::string_view>;

[[nodiscard]] extern
auto type_hint_for_file_extension(std::string file_extension, bool try_all_supported_types) -> expected<AudioTypeHint>;

[[nodiscard]] extern
auto type_hint_for_type(AudioType type, bool try_all_supported_types) -> expected<AudioTypeHint>;

} // blahdio
