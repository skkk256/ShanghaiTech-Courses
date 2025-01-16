#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <map>
#include <memory>
#include <type_traits>

#include "rdr/object.h"
#include "rdr/properties.h"
#include "rdr/std.h"

RDR_NAMESPACE_BEGIN

template <typename IdentifierType>
class BaseFactory {
public:
  using RegisterCallbackType = std::function<void()>;
  using CreateFuncType       = std::function<void *(const Properties &props)>;
  using AssocType            = std::map<IdentifierType, CreateFuncType>;
  using ContextType          = vector<ConfigurableObject *>;

  static BaseFactory &Instance() {  // NOLINT
    static BaseFactory instance;
    return instance;
  }

  /// Reset to as if the factory is just created
  static void clearRuntimeInfo() {
    Instance().callbacks.clear();
    Instance().context.clear();
  }

  /// Register all classes
  static void doRegisterAllClasses() {
    for (const auto &callback : Instance().callbacks) callback();
    Instance().callbacks.clear();
  }

  /// Register a class registration callback to the factory
  /// @note we postpone the registration to the first of main() to deal with
  /// exceptions during static initialization, which might result in weird
  /// behavior
  void registerClassRegisterationCallback(
      const RegisterCallbackType &callback) noexcept(true) {
    callbacks.push_back(callback);
  }

  template <typename ProductType>
  std::enable_if_t<std::is_base_of_v<ConfigurableObject, ProductType>, bool>
  registerClass(const IdentifierType &id, const CreateFuncType &reg) {
    if (registry.find(id) != registry.end())
      Exception_(
          "Identifier [ {} ] already registered, there might be some issues "
          "with your class implementation",
          id);
    registry[id] = reg;
    return true;
  }

  void unregisterClass(const IdentifierType &id) {
    if (registry.find(id) == registry.end()) return;
    registry.erase(id);
  }

  template <typename ProductType = void>
  ref<ProductType> createClass(
      const IdentifierType &id, const Properties &props) {
    if (registry.find(id) == registry.end()) {
      Exception_(
          "Error creating class from factory, identifier [ {} ] not found", id);
      return nullptr;
    }

    void *product = registry[id](props);
    assert(product != nullptr);
    context.push_back(static_cast<ConfigurableObject *>(product));
    return ref<ProductType>(static_cast<ProductType *>(product));
  }

  ContextType &getContext() noexcept { return context; }
  const ContextType &getContext() const noexcept { return context; }

private:
  BaseFactory() = default;

  // we use std::vector instead of project-specific vector to avoid weird
  // behavior during static initialization
  std::vector<RegisterCallbackType> callbacks;

  AssocType registry;
  ContextType context;
};

/// Alias for BaseFactory<std::string>
using Factory = BaseFactory<std::string>;

/**
 * Here we use tricks to ensure that a class might only be registered once
 * Mostly used by TAs, you might not need to use them nor understand them.
 *
 * To whom are interested in the implementation, this uses a series of tricks,
 * 1. tag dispatching for type-deduction and passing template parameter to
 * constructor
 * 2. static variable in template constructor to ensure that the registration is
 * not repeated
 * 3. IIFE(Immediately Invoked Function Expressions) to execute the
 * registeration
 * 4. ODR guaranteed by macro
 * 5. Registeration is postponed into Factory::doRegisterAllClasses() to deal
 * with exceptions
 */
template <typename T>
struct Dummy {};

struct StringRegister {
  template <typename T>
  explicit StringRegister(const std::string &str, Dummy<T>) {
    static bool _ = [=]() -> bool {
      Factory::Instance().registerClass<T>(
          str, [](const Properties &props) { return Memory::alloc<T>(props); });
      return bool{};
    }();
  }
};

struct FactoryRegister {
  template <typename T>
  explicit FactoryRegister(
      const std::string &str, std::function<T *(const Properties &)> reg) {
    static bool _ = [=]() -> bool {
      Factory::Instance().registerClass<T>(str, reg);
      return bool{};
    }();
  }
};

#define RDR_REGISTER_CLASS(T)                                    \
  const static bool __reg_bool_##T = []() -> bool {              \
    Factory::Instance().registerClassRegisterationCallback(      \
        []() -> void { StringRegister dummy(#T, Dummy<T>{}); }); \
    return bool{};                                               \
  }();
#define RDR_REGISTER_FACTORY(T, Func)                                     \
  const static bool __reg_bool_##T = []() -> bool {                       \
    Factory::Instance().registerClassRegisterationCallback([]() -> void { \
      FactoryRegister dummy(                                              \
          #T, static_cast<std::function<T *(const Properties &)>>(Func)); \
    });                                                                   \
    return bool{};                                                        \
  }();
#define RDR_CREATE_CLASS(T, props) Factory::Instance().createClass<T>(#T, props)

RDR_NAMESPACE_END

#endif
