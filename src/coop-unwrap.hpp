#ifndef CUTIL_MACROS_COOP_UNWRAP_HPP
#define CUTIL_MACROS_COOP_UNWRAP_HPP
#include "coop-assert.hpp"
#include "unwrap.hpp"

#define coop_unwrap(var, opt, ...)     generic_unwrap(coop_ensure, const, var, opt, __VA_ARGS__)
#define coop_unwrap_mut(var, opt, ...) generic_unwrap(coop_ensure, /*const*/, var, opt, __VA_ARGS__)

#endif // CUTIL_MACROS_COOP_UNWRAP_HPP
