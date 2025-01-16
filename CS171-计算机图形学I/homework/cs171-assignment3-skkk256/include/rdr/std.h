#ifndef __STD_H__
#define __STD_H__

#include <memory_resource>

#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN

#define RDR_DEFINE_TYPE_ALIAS(name, type) \
  template <typename... T>                \
  using name = type<T...>;

#define RDR_DEFINE_FUNCTION_ALIAS(name, func)                     \
  template <typename... T, typename... Args>                      \
  RDR_FORCEINLINE constexpr decltype(auto) name(Args &&...args) { \
    return func<T...>(std::forward<Args>(args)...);               \
  }

// Only the basic interface is used throughout the code
// RDR_DEFINE_TYPE_ALIAS(ref, std::shared_ptr)
// RDR_DEFINE_FUNCTION_ALIAS(make_ref, std::make_shared)

template <class T>
struct is_unbounded_array : std::false_type {};

template <class T>
struct is_unbounded_array<T[]> : std::true_type {};

template <class T>
constexpr bool is_unbounded_array_v = is_unbounded_array<T>::value;

class Memory {
public:
  Memory()                     = default;
  Memory(Memory &&)            = delete;
  Memory &operator=(Memory &&) = delete;
  Memory(std::pmr::memory_resource *upstream)
      : upstream(upstream), resource(upstream), destructors(upstream) {}
  ~Memory() { release(); }

  Memory(const Memory &)            = delete;
  Memory &operator=(const Memory &) = delete;

  void release() {
    for (auto &i : destructors) delete i;
    destructors.clear();
    resource.release();
  }

  static Memory &Instance() {  // NOLINT
    static Memory instance;
    return instance;
  }

  /// Reset to as if the factory is just created
  static void clearRuntimeInfo() { Instance().release(); }

  /// Use this resource as the upstream
  static std::pmr::memory_resource *asUpstream() {
    return &Instance().resource;
  }

  template <typename... TArgs, typename... Args>
  static decltype(auto) alloc(Args &&...args) {
    return Instance().allocImpl<TArgs...>(std::forward<Args>(args)...);
  }

  void printStatDebug() const {
    Info_("Global Memory Pool: footprint={} KB, allocation_time={}",
        footprint / 1024, allocation_time);
  }

private:
  // Internal Destructor
  struct DestructorBase {
    virtual ~DestructorBase() {}
  };

  template <typename T>
  struct Destructor : public DestructorBase {
    Destructor(T *p) : m_p(p) {}
    ~Destructor() override { std::destroy_at(m_p); }
    T *m_p;
  };

  template <typename T, typename T_ = std::remove_extent_t<T>>
  struct ArrayDestructor : public DestructorBase {
    ArrayDestructor(T_ *p, size_t n) : m_p(p), m_n(n) {}
    ~ArrayDestructor() override { std::destroy_n(m_p, m_n); }
    T_ *m_p;
    size_t m_n;
  };

  using ptr_t = void *;
  std::pmr::memory_resource *upstream{std::pmr::get_default_resource()};
  std::pmr::monotonic_buffer_resource resource{upstream};
  std::pmr::vector<DestructorBase *> destructors{upstream};

  size_t footprint{0};
  size_t allocation_time{0};

  template <typename T, size_t Align = alignof(T), typename... Args>
  RDR_FORCEINLINE std::enable_if_t<!std::is_array<T>::value, T *> allocImpl(
      Args &&...args) {
    auto allocator = std::pmr::polymorphic_allocator<T>(&resource);
    T *mem         = allocator.allocate(sizeof(T));
    assert(((size_t)(void *)mem) % Align == 0);
    allocator.construct(mem, std::forward<Args>(args)...);

    if constexpr (!std::is_trivially_destructible_v<T>)
      destructors.push_back(new Destructor<T>(mem));

    footprint += sizeof(T);
    allocation_time++;
    return mem;
  }

  template <typename T, size_t Align = alignof(T),
      typename T_ = std::remove_extent_t<T>, typename... Args>
  std::enable_if_t<is_unbounded_array_v<T>, T_ *> allocImpl(
      size_t n, Args &&...args) {
    auto allocator = std::pmr::polymorphic_allocator<T_>(&resource);
    T_ *mem        = allocator.allocate(sizeof(T_) * n);
    assert(((size_t)(void *)mem) % Align == 0);
    if constexpr (sizeof...(args) == 0) {
      new (mem) T_[n];
    } else {
      for (size_t i = 0; i < n; ++i)
        allocator.construct(mem + i, std::forward<Args>(args)...);
    }

    if constexpr (!std::is_trivially_destructible_v<T_>)
      destructors.push_back(new ArrayDestructor<T>(mem, n));

    footprint += sizeof(T_) * n;
    allocation_time++;
    return mem;
  }
};

// A naive implementation
// ref means that the lifetime of the object is guaranteed if you owns
// this ref<T>. In our case, it is implemented with a global memory pool for
// simplicity.
template <typename T>
class ref {  // NOLINT
public:
  constexpr ref() = default;
  constexpr ref(T *ptr) : ptr(ptr) {}
  constexpr ref(ref &&other) noexcept : ptr(other.get()) {}
  constexpr ref(const ref &other) : ptr(other.get()) {}

  constexpr ref &operator=(ref other) {
    std::swap(ptr, other.ptr);
    return *this;
  }

  template <typename U,
      typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
  constexpr ref(const ref<U> &other) : ptr(other.get()) {}

  template <typename U,
      typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
  constexpr ref &operator=(const ref<U> &other) {
    ptr = other.get();
    return *this;
  }

  constexpr T *get() { return ptr; }
  constexpr T *get() const { return ptr; }
  constexpr T *operator->() { return ptr; }
  constexpr const T *operator->() const { return ptr; }
  constexpr T &operator*() { return *ptr; }
  constexpr const T &operator*() const { return *ptr; }
  constexpr explicit operator bool() const { return ptr != nullptr; }

  template <typename Other>
  constexpr bool operator==(const Other &other) const {
    return ptr == other;
  }

  template <typename Other>
  constexpr bool operator!=(const Other &other) const {
    return ptr != other;
  }

private:
  T *ptr{nullptr};
};

/// A wrapper similar to WeakPtr, i.e. not interfering
template <typename T>
using ptr = ref<T>;

template <typename T, typename... Args>
RDR_FORCEINLINE constexpr decltype(auto) make_ref(Args &&...args) {  // NOLINT
  return Memory::alloc<T>(std::forward<Args>(args)...);
}

template <typename T>
struct ref_type;

template <typename T>
struct ref_type<ref<T>> {
  using element_type = T;
};

/**
 * @brief Partial implementation of
 * https://en.cppreference.com/w/cpp/container/span, in case the compiler does
 * not support C++ 20.
 *
 * @tparam T
 */
template <typename T>
class span {  // NOLINT
public:
  using element_type    = T;
  using value_type      = std::remove_cv<T>;
  using size_type       = std::size_t;
  using pointer         = T *;
  using const_pointer   = const T *;
  using reference       = T &;
  using const_reference = const T &;

  using iterator         = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;

  constexpr span() noexcept : data_(nullptr), size_(0) {}
  constexpr span(pointer data, size_type size) noexcept
      : data_(data), size_(size) {}
  constexpr span(pointer begin, pointer end) noexcept
      : data_(begin), size_(end - begin) {}
  template <size_type N>
  constexpr span(element_type (&arr)[N]) noexcept : data_(arr), size_(N) {}

  constexpr span(const span &other) noexcept = default;

  constexpr iterator begin() { return data_; }
  constexpr iterator end() { return data_ + size_; }

private:
  pointer data_;
  size_type size_;
};

// A global file resolver initialized with config path
class FileResolver {
public:
  static FileResolver &Instance() {  // NOLINT
    static FileResolver instance;
    return instance;
  }

  /// Set the base path to the absolute directory
  static void setBasePath(const fs::path &path) {
    Instance().base_path = fs::absolute(path);
  }

  /// Reset to as if the factory is just created
  static void clearRuntimeInfo() { Instance().base_path.clear(); }

  /// Given an arbitrary path, resolve it to an absolute path
  static std::string resolveToAbs(const fs::path &path) {
    if (path.is_absolute()) {
      return path.string();
    }

    return (Instance().base_path / path).string();
  }

private:
  fs::path base_path;
};

RDR_NAMESPACE_END

#endif
