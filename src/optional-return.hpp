#pragma once
#include <optional>

#include "assert.hpp"
#include "unwrap.hpp"

// MSVC: inside a trailing-return `-> std::optional<T>` function, the generic `bail`'s
// `return {}` is mis-diagnosed as a void return. A translation unit whose fallible
// functions return std::optional should include this header (after "macros/unwrap.hpp")
// to force the error paths to `return std::nullopt`.
//
// Note: this redefines the macros for the whole translation unit, so any function in the
// same TU that returns a raw pointer must bail manually (`return nullptr`) instead of
// using ensure/unwrap.

#undef bail
#undef ensure
#undef unwrap
#undef unwrap_mut

#define bail(...)                                                                 \
    do {                                                                          \
        CUTIL_MACROS_PRINT_FUNC("assertion failed" __VA_OPT__(": ") __VA_ARGS__); \
        return std::nullopt;                                                      \
    } while(0)

#define ensure(cond, ...) \
    if(!(cond)) {         \
        bail(__VA_ARGS__);\
    }

#define unwrap(var, opt, ...)  \
    const auto var##_o = (opt);\
    if(!(var##_o)) {           \
        return std::nullopt;   \
    }                          \
    const auto& var = *var##_o;

#define unwrap_mut(var, opt, ...) \
    const auto var##_o = (opt);   \
    if(!(var##_o)) {              \
        return std::nullopt;      \
    }                             \
    auto& var = *var##_o;
