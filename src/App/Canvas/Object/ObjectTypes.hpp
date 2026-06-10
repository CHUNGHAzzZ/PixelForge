#pragma once

#include <cstdint>

using ObjectId = std::uint64_t;

inline constexpr ObjectId InvalidObjectId = 0;

enum class ObjectType : std::uint8_t
{
    Unknown = 0,
    Canvas,
    Image,
    Path,
};
