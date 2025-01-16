/**
 * @file platform.h
 * @author ShanghaiTech CS171 TAs
 * @brief Platform-specific (or not) definitions. Contains aliases for output
 * and some basic definitions. Students should respect the interface defined
 * here instead of using their own like `std::cout` which can result in
 * conflicts. Our logging library is `spdlog` and `fmt` for formatting, since
 * they are thread-safe and efficient.
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <fmt/core.h>
#include <fmt/format.h>

#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// ANSICOLOR_SINK is not supported on Windows
#if !defined(_WIN32)
#include <spdlog/sinks/ansicolor_sink.h>
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <optional>

#undef NDEBUG
#include <assert.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#elif defined(__linux__)
#include <sys/ptrace.h>
#endif

#define RDR_NAMESPACE_NAME renderer

#if !defined(RDR_NAMESPACE_BEGIN)
#define RDR_NAMESPACE_BEGIN namespace renderer {
#endif

#if !defined(RDR_NAMESPACE_END)
#define RDR_NAMESPACE_END }  // namespace renderer
#endif

#define RDR_FORCEINLINE inline
#define UNIMPLEMENTED                                                    \
  do {                                                                   \
    Exception_("Unimplemented function at {}:{}\n", __FILE__, __LINE__); \
  } while (false)

template <typename... T>
RDR_FORCEINLINE decltype(auto) print(T &&...args) {
  return fmt::print(std::forward<T>(args)...);
}

template <typename... T>
RDR_FORCEINLINE decltype(auto) format(T &&...args) {
  return fmt::format(std::forward<T>(args)...);
}

// Add suffix to avoid conflicts with other libraries.
struct rdr_exception : public std::runtime_error {  // NOLINT
  using std::runtime_error::runtime_error;
};

#define Info_(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#define Warn_(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#define Error_(...) \
  SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#define Exception_(...)                       \
  do {                                        \
    throw rdr_exception(format(__VA_ARGS__)); \
  } while (false)

namespace fs = std::filesystem;

RDR_NAMESPACE_BEGIN

template <typename T>
using vector = std::vector<T>;

template <typename T>
using optional = std::optional<T>;

RDR_FORCEINLINE void InitLogger(bool b_use_err = false, bool b_quite = false) {
  spdlog::default_logger_raw()->sinks().clear();
#if !defined(_WIN32)
  if (b_use_err)
    spdlog::default_logger_raw()->sinks().push_back(
        std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>());
  else
    spdlog::default_logger_raw()->sinks().push_back(
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
  spdlog::set_pattern("[%^%l%$] %v");
  if (b_quite) spdlog::set_level(spdlog::level::err);
}

RDR_NAMESPACE_END

#endif
