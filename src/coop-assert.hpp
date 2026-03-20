#pragma once
#include <source_location>

#include "assert.hpp"

template <comptime::String func>
constexpr auto coop_detect_error_value() -> auto {
    constexpr auto str000 = func;
    constexpr auto str010 = comptime::remove_prefix<str000, "static ">;
    constexpr auto str020 = comptime::remove_prefix<str010, "virtual ">;
    constexpr auto str030 = comptime::remove_prefix<str020, "const ">;
#ifdef _MSC_VER
    // MSVC __FUNCSIG__ resolves type aliases, so coop::Async<T> appears as
    // coop::CoGenerator<T>. Format: "struct coop::CoGenerator<bool> __cdecl func(...)"
    constexpr auto str040 = comptime::remove_prefix<str030, "class ">;
    constexpr auto str050 = comptime::remove_prefix<str040, "struct ">;
    // Try both the alias and the underlying type
    constexpr auto marker_async = comptime::String("coop::Async<");
    constexpr auto marker_cogen = comptime::String("coop::CoGenerator<");
    constexpr auto open_async = comptime::find<str050, marker_async>;
    constexpr auto open_cogen = comptime::find<str050, marker_cogen>;
    constexpr auto open = (open_async != std::string_view::npos) ? open_async : open_cogen;
    constexpr auto marker_len = (open_async != std::string_view::npos) ? marker_async.size() : marker_cogen.size();
    if constexpr(open == std::string_view::npos) {
        return;
    } else {
        constexpr auto inner_start = open + marker_len;
        constexpr auto close = comptime::find<str050, "> ", inner_start>;
        if constexpr(close == std::string_view::npos) {
            return;
        } else {
            constexpr auto ret = comptime::substr<str050, inner_start, close - inner_start>;
            // Strip MSVC elaborated type specifiers from inner type
            constexpr auto ret2 = comptime::remove_prefix<ret, "class ">;
            constexpr auto ret3 = comptime::remove_prefix<ret2, "struct ">;
            constexpr auto ret4 = comptime::remove_prefix<ret3, "enum ">;
            if constexpr(comptime::find<ret4, "*"> != std::string_view::npos) {
                return nullptr;
            } else {
                constexpr auto bare_ret = comptime::remove_region<ret4, '<', '>'>;
                return type_string_to_type<bare_ret>();
            }
        }
    }
#else
    constexpr auto marker = comptime::String("coop::Async<");
    constexpr auto open   = comptime::find<str030, marker>;
    if constexpr(open == std::string_view::npos) {
        return;
    } else {
        constexpr auto region = comptime::find_region<str030, '<', '>'>;
        if constexpr(region.first == std::string_view::npos) {
            return;
        } else {
            constexpr auto ret = comptime::substr<str030, region.first + 1, region.second - 2>;
            if constexpr(ret[-1] == '*') {
                return nullptr;
            } else {
                constexpr auto str000 = comptime::remove_region<ret, '<', '>'>;
                // gcc adds weird ' ' after template parameters
                constexpr auto str010 = comptime::remove_suffix<str000, " ">;
                return type_string_to_type<str010>();
            }
        }
    }
#endif
}

#ifdef _MSC_VER
#define coop_bail(...)                    \
    CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__); \
    co_return coop_detect_error_value<CUTIL_COMPSTR(__FUNCSIG__)>();
#else
#define coop_bail(...)                    \
    CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__); \
    co_return coop_detect_error_value<CUTIL_COMPSTR(std::source_location::current().function_name())>();
#endif

#define coop_ensure(cond, ...)                                      \
    if(!(cond)) {                                                   \
        coop_bail("assertion failed" __VA_OPT__(": ") __VA_ARGS__); \
    }
