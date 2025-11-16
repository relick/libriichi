#pragma once

#include <array>
#include <optional>
#include <span>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

//------------------------------------------------------------------------------
// Using std containers is fine, but avoid directly using them so we can
// easily refactor to different containers later
//------------------------------------------------------------------------------
namespace Riichi
{

template<typename T, size_t S>
using Array = std::array<T, S>;

template<typename T>
using Option = std::optional<T>;

template<typename K, typename V>
using Map = std::unordered_map<K, V>;

template<typename T, typename S>
using Pair = std::pair<T, S>;

template<typename T, typename Pred = std::equal_to<T>>
using Set = std::unordered_set<T, std::hash<T>, Pred>;

template<typename... Ts>
using Union = std::variant<Ts...>;

template<typename T>
using Vector = std::vector<T>;

template<typename T, std::size_t Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

template<typename... Ts>
using Tuple = std::tuple<Ts...>;

}