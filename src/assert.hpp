#pragma once
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
#ifdef _MSC_VER
    // MSVC __FUNCSIG__ format: "RetType __cdecl FuncName(Args)"
    // Strip class/struct elaborated type specifiers from return type
    constexpr auto str040 = comptime::remove_prefix<str030, "class ">;
    constexpr auto str050 = comptime::remove_prefix<str040, "struct ">;
    constexpr auto str060 = comptime::remove_prefix<str050, "enum ">;
    // Find calling convention as separator between return type and function name
    constexpr auto cdecl_pos = comptime::find<str060, "__cdecl ">;
    constexpr auto thiscall_pos = comptime::find<str060, "__thiscall ">;
    constexpr auto sep = (cdecl_pos != std::string_view::npos) ? cdecl_pos : thiscall_pos;
    if constexpr(sep == std::string_view::npos) {
        return;
    } else {
        constexpr auto ret_with_space = comptime::substr<str060, 0, sep>;
        constexpr auto ret = comptime::remove_suffix<ret_with_space, " ">;
        constexpr auto has_star = comptime::find<ret, "*"> != std::string_view::npos;
        if constexpr(has_star) {
            return nullptr;
        } else {
            constexpr auto bare_ret = comptime::remove_region<ret, '<', '>'>;
            return type_string_to_type<bare_ret>();
        }
    }
#else
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
#endif
}

#ifdef _MSC_VER
#define bail(...)                         \
    CUTIL_MACROS_PRINT_FUNC(__VA_ARGS__); \
    return detect_error_value<CUTIL_COMPSTR(__FUNCSIG__)>();
#else
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

// returns explicitly specified variable on error
// constexpr auto error_value = (-1, std::nullopt, nullptr, ...)
#define bail_v(...)         generic_bail(return return_error_v(error_value), __VA_ARGS__)
#define ensure_v(cond, ...) generic_ensure(bail_v, cond, __VA_ARGS__)

// coroutine version
#define co_bail_v(...)         generic_bail(co_return return_error_v(error_value), __VA_ARGS__)
#define co_ensure_v(cond, ...) generic_ensure(co_bail_v, cond, __VA_ARGS__)

// executes action on error
#define bail_a(...)         generic_bail(error_act, __VA_ARGS__)
#define ensure_a(cond, ...) generic_ensure(bail_a, cond, __VA_ARGS__)
