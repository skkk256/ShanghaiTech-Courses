/**
 * @file canary.h
 * @author ShanghaiTech CS171 TAs
 * @brief Our *Canary system* to guard numerical errors. Its usage is
 * self-explanatory and simple. You don't have to understand its implementation.
 * @version 0.1
 * @date 2023-07-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CANARY_H__
#define __CANARY_H__

#include <numeric>
#include <type_traits>

#include "rdr/math_aliases.h"
#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN
namespace detail_ {
template <typename T>
struct IsValid {
  bool operator()(const T &value) {
    return !std::isnan(value) && !std::isinf(value);
  }
};

template <typename T>
struct IsPositive {
  bool operator()(const T &value) { return value > 0; }
};

template <typename T>
struct IsNonNegative {
  bool operator()(const T &value) { return value >= 0; }
};

template <typename T>
struct IsNear {
  IsNear() = delete;
  IsNear(const T &in_b, const T &in_eps) : b(in_b), eps(in_eps) {}
  bool operator()(const T &a) { return std::abs(a - b) <= eps; }

private:
  const T b;
  const T eps;
};

template <typename T>
struct IsNormalized {
  std::enable_if_t<is_vec_type_v<T>, bool> operator()(const T &value) {
    using value_type = typename vec_type<T>::value_type;
    return IsNear<value_type>(1, NORMAL_EPS)(SquareNorm(value));
  }
};
}  // namespace detail_

template <template <typename> typename UnaryPredicate, typename T,
    typename... Args>
RDR_FORCEINLINE bool IsAllUnaryElementwise(const T &head, const Args &...tail) {
  bool result = true;
  if constexpr (std::is_floating_point_v<T>) {
    result &= UnaryPredicate<T>()(head);
  } else if constexpr (is_vec_type_v<T>) {
    using value_type = typename vec_type<T>::value_type;
    if constexpr (std::is_floating_point_v<value_type>) {
      constexpr int M = vec_type<T>::size;
      for (int i = 0; i < M; ++i)
        result &= UnaryPredicate<value_type>()(head[i]);
    }
  }

  if constexpr (sizeof...(tail) == 0) {
    return result;
  } else {
    return result & IsAllUnaryElementwise<UnaryPredicate>(tail...);
  }
}

template <typename T, typename... Args>
RDR_FORCEINLINE bool IsAllValid(const T &head, const Args &...tail) {
  return IsAllUnaryElementwise<detail_::IsValid>(head, tail...);
}

template <typename T, typename... Args>
RDR_FORCEINLINE bool IsAllPositive(const T &head, const Args &...tail) {
  return IsAllUnaryElementwise<detail_::IsPositive>(head, tail...);
}

template <typename T, typename... Args>
RDR_FORCEINLINE bool IsAllNonNegative(const T &head, const Args &...tail) {
  return IsAllUnaryElementwise<detail_::IsNonNegative>(head, tail...);
}

template <template <typename> typename UnaryPredicate, typename T,
    typename... Args>
RDR_FORCEINLINE bool IsAllUnaryAggregate(const T &head, const Args &...tail) {
  bool result = UnaryPredicate<T>()(head);

  if constexpr (sizeof...(tail) == 0) {
    return result;
  } else {
    return result & IsAllUnaryAggregate<UnaryPredicate>(tail...);
  }
}

template <typename T, typename... Args>
RDR_FORCEINLINE bool IsAllNormalized(const T &head, const Args &...tail) {
  return IsAllUnaryAggregate<detail_::IsNormalized>(head, tail...);
}

template <typename T, typename U = Float>
RDR_FORCEINLINE bool IsNear(const T &a, const T &b, const U &eps = EPS) {
  bool result = true;
  if constexpr (std::is_floating_point_v<T>) {
    result &= (std::abs(a - b) < eps);
  } else if constexpr (is_vec_type_v<T>) {
    using value_type = typename vec_type<T>::value_type;
    if constexpr (std::is_floating_point_v<value_type>) {
      constexpr int M = vec_type<T>::size;
      for (int i = 0; i < M; ++i) result &= IsNear(a[i], b[i], eps);
    }
  }

  return result;
}

template <typename T, typename U = Float>
RDR_FORCEINLINE bool IsGreaterThan(const T &a, const U &b) {
  bool result = true;
  if constexpr (std::is_arithmetic_v<T>) {
    result = (a > b);
  } else if constexpr (is_vec_type_v<T>) {
    using value_type = typename vec_type<T>::value_type;
    if constexpr (std::is_floating_point_v<value_type>) {
      constexpr int M = vec_type<T>::size;
      for (int i = 0; i < M; ++i) result &= (a[i] > b[i]);
    }
  }

  return result;
}

#define AssertAllValid(...)                                         \
  do {                                                              \
    if (!IsAllValid(__VA_ARGS__))                                   \
      Error_("AssertAllValid failed at {}:{}", __FILE__, __LINE__); \
  } while (false)

#define AssertAllPositive(...)                                         \
  do {                                                                 \
    if (!IsAllPositive(__VA_ARGS__))                                   \
      Error_("AssertAllPositive failed at {}:{}", __FILE__, __LINE__); \
  } while (false)

#define AssertAllNonNegative(...)                                         \
  do {                                                                    \
    if (!IsAllNonNegative(__VA_ARGS__))                                   \
      Error_("AssertAllNonNegative failed at {}:{}", __FILE__, __LINE__); \
  } while (false)

#define AssertNear(...)                                         \
  do {                                                          \
    if (!IsNear(__VA_ARGS__))                                   \
      Error_("AssertNear failed at {}:{}", __FILE__, __LINE__); \
  } while (false)

#define AssertAllNormalized(...)                                         \
  do {                                                                   \
    if (!IsAllNormalized(__VA_ARGS__))                                   \
      Error_("AssertAllNormalized failed at {}:{}", __FILE__, __LINE__); \
  } while (false)

RDR_NAMESPACE_END

#endif
