#ifndef CUTIL_MACROS_ASSERT_HPP
#define CUTIL_MACROS_ASSERT_HPP
#include <optional>
#include <source_location>

#include "print.hpp"
#include "util/assert.hpp"

#ifndef CUTIL_MACROS_PRINT_FUNC
#define CUTIL_MACROS_PRINT_FUNC WARN
#endif

#define PANIC(...)                                                       \
    CUTIL_MACROS_PRINT_FUNC("fatal error" __VA_OPT__(": ") __VA_ARGS__); \
    panic("");

#define ASSERT(cond, ...)   \
    if(!(cond)) {           \
        PANIC(__VA_ARGS__); \
    }

//
// returns automatically detected error value
//

#ifdef _MSC_VER
// MSVC's function signatures aren't parseable the way the generic detector (the
// non-MSVC branch below) expects, so derive the error value from __FUNCSIG__ directly.
// MSVC renders the return type first, e.g. "void __cdecl ns::f(...)" or
// "class std::optional<T> __cdecl ns::f(...)". We special-case the two return types
// where a list-initialized `return {}` is wrong or ill-formed:
//   - void                -> `return;`           (`return {}` is ill-formed for void)
//   - std::optional<...>  -> `return std::nullopt;` (MSVC mis-diagnoses `return {}`
//                                                    here as a void return)
// everything else (pointer / bool / numeric / containers) accepts `return {}`.
// This makes bail type-correct per-function, so a TU may freely mix return types
// (e.g. string-reader's optional readers alongside its bool helpers).
template <comptime::String sig>
constexpr auto msft_sig_is_void_fn() -> bool {
    return comptime::starts_with<sig, "void __cdecl "> ||
           comptime::starts_with<sig, "void __stdcall "> ||
           comptime::starts_with<sig, "void __fastcall ">;
}

template <comptime::String sig>
constexpr auto msft_sig_returns_optional() -> bool {
    return comptime::starts_with<sig, "class std::optional<">;
}

#define bail(...)                                                                      \
    do {                                                                               \
        CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__);                                          \
        if constexpr(msft_sig_is_void_fn<CUTIL_COMPSTR(__FUNCSIG__)>()) {              \
            return;                                                                    \
        } else if constexpr(msft_sig_returns_optional<CUTIL_COMPSTR(__FUNCSIG__)>()) { \
            return std::nullopt;                                                       \
        } else {                                                                       \
            return {};                                                                 \
        }                                                                              \
    } while(0)
#else
template <comptime::String str>
constexpr auto type_string_to_type() -> auto {
    if constexpr(str.str() == "std::unique_ptr" || str.str() == "std::shared_ptr") {
        return nullptr;
    } else if constexpr(str.str() == "void") {
        return;
    } else if constexpr(str.str() == "bool") {
        return false;
    } else if constexpr(str.str() == "int") {
        return -1;
    } else if constexpr(str.str() == "std::optional") {
        return std::nullopt;
    } else {
        return;
    }
}

template <comptime::String func>
constexpr auto detect_error_value() -> auto {
    constexpr auto str000 = func;
    constexpr auto str010 = comptime::remove_prefix<str000, "static ">;
    constexpr auto str020 = comptime::remove_prefix<str010, "virtual ">;
    constexpr auto str030 = comptime::remove_prefix<str020, "const ">;
    constexpr auto str040 = comptime::remove_region<str030, '<', '>'>;
    constexpr auto space  = comptime::find<str040, " ">;
    if constexpr(space == std::string_view::npos) {
        return;
    } else {
        constexpr auto ret  = comptime::substr<str040, 0, space>;
        constexpr auto name = comptime::substr<str040, space + 1>;
        if constexpr(ret[-1] == '*' || name[0] == '*') {
            return nullptr;
        } else {
            return type_string_to_type<ret>();
        }
    }
}

#define bail(...)                         \
    CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__); \
    return detect_error_value<CUTIL_COMPSTR(std::source_location::current().function_name())>();
#endif

#define ensure(cond, ...)                                      \
    if(!(cond)) {                                              \
        bail("assertion failed" __VA_OPT__(": ") __VA_ARGS__); \
    }

//
// manual action
//

struct VoidErrorType {};

template <class T>
constexpr auto return_error_v(T error_value) -> T {
    return error_value;
}

constexpr auto return_error_v(VoidErrorType) -> void {
    return;
}

constexpr auto error_value = VoidErrorType{};

#define generic_bail(error_act, ...)                      \
    {                                                     \
        __VA_OPT__(CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__);) \
        error_act;                                        \
    }
#define generic_ensure(bail, cond, ...)                        \
    if(!(cond)) {                                              \
        bail("assertion failed" __VA_OPT__(": ") __VA_ARGS__); \
    }

#define bail_v(...)         generic_bail(return return_error_v(error_value), __VA_ARGS__)
#define ensure_v(cond, ...) generic_ensure(bail_v, cond, __VA_ARGS__)

#define co_bail_v(...)         generic_bail(co_return return_error_v(error_value), __VA_ARGS__)
#define co_ensure_v(cond, ...) generic_ensure(co_bail_v, cond, __VA_ARGS__)

#define bail_a(...)         generic_bail(error_act, __VA_ARGS__)
#define ensure_a(cond, ...) generic_ensure(bail_a, cond, __VA_ARGS__)

#endif // CUTIL_MACROS_ASSERT_HPP
