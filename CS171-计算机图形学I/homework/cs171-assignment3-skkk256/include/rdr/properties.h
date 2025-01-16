/**
 * @file integrator.h
 * @author ShanghaiTech CS171 TAs
 * @brief This provides a Properties class to conveniently construct all
 * subclasses of ConfigurableObject and manage all parameters a class has.
 * @version 0.1
 * @date 2023-04-30
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <map>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <variant>

#include "rdr/math_aliases.h"
#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN

/**
 * @class Properties
 * @brief Associative parameter map for constructing subclasses of @ref
 * ConfigurableObject.
 * @details
 * In general, *Properties* is a section surrounded by "{}" in json.
 * Our properties supports a subset of json format. A json file follows the
 * pattern of [name]: type, while type can be bool; int; Float; Vectors; string;
 * yet another property; and an array of property. The root node does not have a
 * name.

 * For efficiency and simplicity, a vector should contain only elements of
 * a single type. The type can be bool; int; Float; Properties
 */
class Properties {
public:
  using PropertyElement = std::variant<bool, int, Float, Vec2f, Vec2i, Vec3f,
      Vec3i, Mat4f, std::string, Properties, vector<Properties>>;

  using json = ::nlohmann::json;

  /// Construct an empty Propertis
  Properties() = default;

  /// Copy constructor
  Properties(const Properties &props) = default;

  /// Construct from json
  Properties(const json &j);

  /// Move constructor
  Properties(Properties &&props) noexcept
      : properties_map(std::move(props.properties_map)) {}

  /// Release all memory
  ~Properties() = default;

  /**
   * @brief Check if a property name exists
   */
  bool hasProperty(const std::string &name) const {
    return properties_map.find(name) != properties_map.end();
  }

  /**
   * @brief Get property value from the given name with default value.
   */
  template <typename PropertyType>
  PropertyType getProperty(const std::string &name) const {
    if (properties_map.find(name) == properties_map.end()) {
      Exception_("Cannot find property with the name [ {} ]", name);
    }

    const auto &element = properties_map.find(name)->second;
    try {
      return implicitCast<PropertyType>(element);
    } catch (std::bad_variant_access const &ex) {
      Exception_(
          "The type of the acquired property [ {} ] is not matched."
          "And implicit cast is not supported for now: {}",
          name, ex.what());
    }
  }

  template <typename PropertyType>
  PropertyType getProperty(
      const std::string &name, PropertyType def_val) const {
    if (properties_map.find(name) == properties_map.end()) {
      if constexpr (fmt::is_formattable<PropertyType>::value)
        Warn_("Property [ {} ] not found, use the default value [ {} ]", name,
            def_val);
      else
        Warn_("Property [ {} ] not found, use the default value", name);
      return def_val;
    }

    const auto &element = properties_map.find(name)->second;
    try {
      return implicitCast<PropertyType>(element);
    } catch (std::bad_variant_access const &ex) {
      Exception_(
          "The type of the acquired property [ {} ] is not matched."
          "And implicit cast is not supported for now: {}",
          name, ex.what());
    }
  }

  /**
   * @brief Set property with given name to the give value.
   */
  template <typename PropertyType>
  void setProperty(const std::string &name, const PropertyType &value) {
    if (properties_map.find(name) != properties_map.end())
      Warn_("Property [ {} ] was specified for multiple times", name.c_str());
    properties_map[name] = value;
  }

  /**
   * @brief Get a reference to property element from the given name with default
   * value.
   * @warning Once this method is called, even though you don't assign value to
   * it, there will be a property with name be created.
   */
  PropertyElement &operator[](const std::string &name) {
    return properties_map[name];
  }

  /// Copy assignment
  Properties &operator=(const Properties &rhs) = default;

  /// Move assignment
  Properties &operator=(Properties &&rhs) noexcept {
    properties_map = std::move(rhs.properties_map);
    return *this;
  }

  /// Equality comparison
  bool operator==(const Properties &rhs) const {
    return properties_map == rhs.properties_map;
  }

  /// Ineqaulity comparision
  bool operator!=(const Properties &rhs) const {
    return properties_map != rhs.properties_map;
  }

  // add simple iterator interface to expose the propertiesMap
  auto begin() { return properties_map.begin(); }
  auto end() { return properties_map.end(); }
  auto begin() const { return properties_map.begin(); }
  auto end() const { return properties_map.end(); }

  /// Convert to a json-like format
  std::string toString() const {
    return toStringInternal(0, false, false, false);
  }

  /// Print all properties
  void printDebug(int offset = 0, bool has_first_indent = false,
      bool has_tail_coma = false) const {
    print("{}\n", toStringInternal(offset, has_first_indent, has_tail_coma));
  }

  /// Clear all properties for debugging propose (debugger will see a lot of
  /// properties if not cleared, and they are only used on crossConfiguration
  /// phase)
  void clear() { properties_map.clear(); }

private:
  std::map<std::string, PropertyElement> properties_map;

  std::string toStringInternal(int offset = 0, bool has_first_indent = false,
      bool has_tail_coma = false, bool has_tail_newline = false) const;

  template <typename T>
  T implicitCast(const PropertyElement &elem) const {
    if constexpr (std::is_same_v<T, Float>) {
      // Can be implicitly casted from int
      if (std::holds_alternative<Float>(elem))
        return std::get<Float>(elem);
      else if (std::holds_alternative<int>(elem))
        return static_cast<Float>(std::get<int>(elem));
    } else if constexpr (is_vec_type_v<T>) {
      // Can be implicitly cased from Vec<int>
      using VecTypeTrait = vec_type<T>;
      if constexpr (std::is_same_v<typename VecTypeTrait::value_type, Float>) {
        using VecType =
            Vec<typename VecTypeTrait::value_type, VecTypeTrait::size>;
        if (std::holds_alternative<VecType>(elem))
          return std::get<VecType>(elem);
        else if (std::holds_alternative<Vec<int, VecTypeTrait::size>>(elem))
          return Cast<Float>(std::get<Vec<int, VecTypeTrait::size>>(elem));
      }
    }

    return std::get<T>(elem);
  }
};

RDR_NAMESPACE_END

#endif
