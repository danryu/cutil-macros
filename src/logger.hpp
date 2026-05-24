#ifndef CUTIL_MACROS_LOGGER_HPP
#define CUTIL_MACROS_LOGGER_HPP
#include "print.hpp"
#include "util/logger.hpp"

#define CUTIL_GENERIC_LOG_PRINT(level, logger, ...)                 \
    {                                                               \
        constexpr auto loc  = std::source_location::current();      \
        constexpr auto file = CUTIL_COMPSTR(loc.file_name());       \
        constexpr auto func = CUTIL_COMPSTR(loc.function_name());   \
        constexpr auto line = loc.line();                           \
        logger_print<file, func, line, level>(logger, __VA_ARGS__); \
    }

#define LOG_ERROR(logger, ...) CUTIL_GENERIC_LOG_PRINT(Loglevel::Error, logger, __VA_ARGS__)
#define LOG_WARN(logger, ...)  CUTIL_GENERIC_LOG_PRINT(Loglevel::Warn, logger, __VA_ARGS__)
#define LOG_DEBUG(logger, ...) CUTIL_GENERIC_LOG_PRINT(Loglevel::Debug, logger, __VA_ARGS__)
#define LOG_INFO(logger, ...)  CUTIL_GENERIC_LOG_PRINT(Loglevel::Info, logger, __VA_ARGS__)

#endif // CUTIL_MACROS_LOGGER_HPP
