#pragma once

#include <string>
#include <string_view>
#include <unordered_map>


namespace stl_extra
{
    struct string_equal {
        using is_transparent = std::true_type;

        bool operator()(std::string_view l, std::string_view r) const noexcept
        {
            return l == r;
        }
    };

    struct string_hash {
        using is_transparent = std::true_type;

        auto operator()(std::string_view str) const noexcept {
            return std::hash<std::string_view>()(str);
        }
    };

    // Specialized `std::unordered_map` with ability to search key using `std::string_view`:
    // Solution original URL: https://stackoverflow.com/a/64101153/739731
    template <typename Value>
    using string_unordered_map = std::unordered_map<std::string, Value, string_hash, string_equal>;
}
