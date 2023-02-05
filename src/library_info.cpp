#include <algorithm>
#include <fmt/format.h>
#include "blahdio/library_info.h"

using namespace std::literals::string_view_literals;

namespace blahdio {

struct TypeInfo
{
	AudioType type;
	std::string_view file_extension;
	AudioTypeHint hint_try_all;
	AudioTypeHint hint_try_only;
};

using Table = std::array<TypeInfo, 4>;

template <typename Predicate> [[nodiscard]] constexpr static
auto find(const Table& table, Predicate predicate)
{
	return std::find_if(begin(table), end(table), predicate);
}

[[nodiscard]] constexpr static
auto find(const Table& table, AudioType type)
{
	return find(table, [type](const TypeInfo& info)
	{
		return info.type == type;
	});
}

[[nodiscard]] constexpr static
auto find(const Table& table, std::string_view file_extension)
{
	return find(table, [file_extension](const TypeInfo& info)
	{
		return info.file_extension == file_extension;
	});
}

static constexpr Table TABLE =
{{
	{ AudioType::flac,    "FLAC"sv, AudioTypeHint::try_flac_first,    AudioTypeHint::try_flac_only },
	{ AudioType::mp3,     "MP3"sv,  AudioTypeHint::try_mp3_first,     AudioTypeHint::try_mp3_only },
	{ AudioType::wav,     "WAV"sv,  AudioTypeHint::try_wav_first,     AudioTypeHint::try_wav_only },
	{ AudioType::wavpack, "WV"sv,   AudioTypeHint::try_wavpack_first, AudioTypeHint::try_wavpack_only }
}};

[[nodiscard]] extern constexpr
auto get_file_extension(AudioType type) -> std::string_view
{
	const auto pos{find(TABLE, type)};

	if (pos == TABLE.end()) return "";

	return pos->file_extension;
}

[[nodiscard]]
auto known_file_extensions() -> std::vector<std::string_view>
{
	std::vector<std::string_view> out;

	std::transform(TABLE.begin(), TABLE.end(), std::back_inserter(out), [](const TypeInfo& info)
	{
		return info.file_extension;
	});

	return out;
}

[[nodiscard]] constexpr
auto get_type_hint(const TypeInfo& info, bool try_all_supported_types) -> AudioTypeHint
{
	return try_all_supported_types ? info.hint_try_all : info.hint_try_only;
}

[[nodiscard]]
auto type_hint_for_file(std::filesystem::path file_path, bool try_all_supported_types) -> expected<AudioTypeHint>
{
	const auto toupper = [](std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](char c) { return std::toupper(c); });
		return str;
	};

	const auto ext{file_path.extension().string()};
	const auto upper_ext{toupper(ext)};
	const auto pos{find(TABLE, std::string_view{ext})};

	if (pos == TABLE.end())
	{
		return tl::make_unexpected(fmt::format("Unrecognized file extension: {}", ext));
	}

	return get_type_hint(*pos, try_all_supported_types);
}

[[nodiscard]]
auto type_hint_for_type(AudioType type, bool try_all_supported_types) -> expected<AudioTypeHint>
{
	const auto pos{find(TABLE, type)};

	if (pos == TABLE.end())
	{
		return tl::make_unexpected(fmt::format("Invalid audio type"));
	}

	return get_type_hint(*pos, try_all_supported_types);
}

} // blahdio
