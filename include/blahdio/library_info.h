#pragma once

#include <array>
#include <filesystem>
#include "expected.h"
#include "audio_type.h"

namespace blahdio {

[[nodiscard]] extern
auto get_file_extension(AudioType type) -> std::string_view;

[[nodiscard]] extern
auto known_file_extensions() -> std::vector<std::string_view>;

[[nodiscard]] extern
auto type_hint_for_file(std::filesystem::path file_path, bool try_all_supported_types) -> expected<AudioTypeHint>;

[[nodiscard]] extern
auto type_hint_for_type(AudioType type, bool try_all_supported_types) -> expected<AudioTypeHint>;

} // blahdio
