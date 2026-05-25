#ifndef CUTIL_MACROS_COOP_ASSERT_HPP
#define CUTIL_MACROS_COOP_ASSERT_HPP
#include <source_location>

#include "assert.hpp"

#ifdef _MSC_VER
template <comptime::String sig>
constexpr auto msft_sig_has_trailing_void_return() -> bool {
    if constexpr(comptime::ends_with<sig, "-> void">) {
        return true;
    }
    if constexpr(comptime::find<sig, "-> void "> != std::string_view::npos) {
        return true;
    }
    if constexpr(comptime::find<sig, "-> void__ptr64"> != std::string_view::npos) {
        return true;
    }
    return false;
}

template <comptime::String sig>
constexpr auto msft_sig_is_void_coop() -> bool {
    if constexpr(comptime::find<sig, "coop::Async<void>"> != std::string_view::npos) {
        return true;
    }
    if constexpr(comptime::find<sig, "coop::Async<void >"> != std::string_view::npos) {
        return true;
    }
    if constexpr(comptime::find<sig, "coop::CoGenerator<void>"> != std::string_view::npos) {
        return true;
    }
    if constexpr(comptime::find<sig, "coop::CoGenerator<void >"> != std::string_view::npos) {
        return true;
    }
    if constexpr(msft_sig_has_trailing_void_return<sig>()) {
        return true;
    }
    return msft_sig_is_void_fn<sig>();
}

// Mirror of msft_sig_returns_optional (in assert.hpp) for coroutines: detect
// coop::Async<std::optional<...>> / coop::CoGenerator<std::optional<...>> so coop_bail
// can co_return std::nullopt instead of an ill-diagnosed co_return {}.
template <comptime::String sig>
constexpr auto msft_sig_is_optional_coop() -> bool {
    return comptime::find<sig, "coop::Async<class std::optional<"> != std::string_view::npos ||
           comptime::find<sig, "coop::CoGenerator<class std::optional<"> != std::string_view::npos;
}

#define coop_bail(...)                                                                          \
    do {                                                                                          \
        CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__);                                                     \
        if constexpr(msft_sig_is_void_coop<CUTIL_COMPSTR(__FUNCSIG__)>()) {                        \
            co_return;                                                                            \
        } else if constexpr(msft_sig_is_optional_coop<CUTIL_COMPSTR(__FUNCSIG__)>()) {             \
            co_return std::nullopt;                                                               \
        } else {                                                                                  \
            co_return {};                                                                           \
        }                                                                                         \
    } while(0)

#else
template <comptime::String func>
constexpr auto coop_detect_error_value() -> auto {
    constexpr auto str000 = func;
    constexpr auto str010 = comptime::remove_prefix<str000, "static ">;
    constexpr auto str020 = comptime::remove_prefix<str010, "virtual ">;
    constexpr auto str030 = comptime::remove_prefix<str020, "const ">;
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
}

#define coop_bail(...)                    \
    CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__); \
    co_return coop_detect_error_value<CUTIL_COMPSTR(std::source_location::current().function_name())>();

#endif

#define coop_ensure(cond, ...)                                      \
    if(!(cond)) {                                                   \
        coop_bail("assertion failed" __VA_OPT__(": ") __VA_ARGS__); \
    }

#endif // CUTIL_MACROS_COOP_ASSERT_HPP
