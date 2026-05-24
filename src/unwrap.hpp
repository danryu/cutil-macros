#ifndef CUTIL_MACROS_UNWRAP_HPP
#define CUTIL_MACROS_UNWRAP_HPP
#include "assert.hpp"

#define generic_unwrap(ensure, const, var, opt, ...) \
    const auto var##_o = opt;                        \
    ensure(var##_o, __VA_ARGS__);                    \
    const auto& var = *var##_o;

#define generic_unwrap_v(ensure_v, const, var, opt, ...) \
    const auto var##_o = opt;                            \
    ensure_v(var##_o, __VA_ARGS__);                      \
    const auto& var = *var##_o;

#define generic_unwrap_a(ensure_v, const, var, opt, ...) \
    const auto var##_o = opt;                            \
    ensure_a(var##_o, __VA_ARGS__);                      \
    const auto& var = *var##_o;

#define unwrap(var, opt, ...)          generic_unwrap(ensure, const, var, opt, __VA_ARGS__)
#define unwrap_mut(var, opt, ...)      generic_unwrap(ensure, /*const*/, var, opt, __VA_ARGS__)
#define unwrap_v(var, opt, ...)        generic_unwrap_v(ensure_v, const, var, opt, __VA_ARGS__)
#define unwrap_v_mut(var, opt, ...)    generic_unwrap_v(ensure_v, /*const*/, var, opt, __VA_ARGS__)
#define unwrap_a(var, opt, ...)        generic_unwrap_a(ensure_v, const, var, opt, __VA_ARGS__)
#define unwrap_a_mut(var, opt, ...)    generic_unwrap_a(ensure_v, /*const*/, var, opt, __VA_ARGS__)
#define co_unwrap_v(var, opt, ...)     generic_unwrap_v(co_ensure_v, const, var, opt, __VA_ARGS__)
#define co_unwrap_v_mut(var, opt, ...) generic_unwrap_v(co_ensure_v, /*const*/, var, opt, __VA_ARGS__)

#endif // CUTIL_MACROS_UNWRAP_HPP
