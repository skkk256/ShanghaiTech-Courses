#include "rdr/properties.h"

#include <functional>
#include <nlohmann/json.hpp>

RDR_NAMESPACE_BEGIN

static bool isNumericArray(const ::nlohmann::json &j) {
  return std::all_of(j.begin(), j.end(),
      [](const ::nlohmann::json &x) { return x.is_number(); });
}

static bool isFloatArray(const ::nlohmann::json &j) {
  return isNumericArray(j) &&
         std::any_of(j.begin(), j.end(),
             [](const ::nlohmann::json &x) { return x.is_number_float(); });
}

static bool isObjectArray(const ::nlohmann::json &j) {
  return std::all_of(j.begin(), j.end(),
      [](const ::nlohmann::json &x) { return x.is_object(); });
}

Properties::Properties(const json &j) {
  std::function<Properties(const json &)> recursive_construct;
  recursive_construct = [&](const json &j) -> Properties {
    Properties result;
    for (auto &[name, property] : j.items()) {
      switch (property.type()) {
        case json::value_t::object:
          result[name] = recursive_construct(property);
          break;
        case json::value_t::string:
          result[name] = property.get<std::string>();
          break;
        case json::value_t::boolean:
          result[name] = property.get<bool>();
          break;
        case json::value_t::number_unsigned:
        case json::value_t::number_integer:
          result[name] = property.get<int>();
          break;
        case json::value_t::number_float:
          result[name] = property.get<Float>();
          break;
        case json::value_t::array:
          /**
           * Here is why we say that Properties is a subset of json.
           * Array of numbers are specially dealt with.
           * The definition of *array* is limited. (must contain all objects or
           * numbers)
           */
          if (isNumericArray(property)) {
            if (property.size() != 2 && property.size() != 3 &&
                property.size() != 16) {
              Exception_("json array size not matched");
            }

            if (isFloatArray(property)) {
              if (property.size() == 2)
                result[name] = Vec2f(property[0], property[1]);
              else if (property.size() == 3)
                result[name] = Vec3f(property[0], property[1], property[2]);
              else
                result[name] = Mat4f({
                    {property[0], property[4],  property[8], property[12]},
                    {property[1], property[5],  property[9], property[13]},
                    {property[2], property[6], property[10], property[14]},
                    {property[3], property[7], property[11], property[15]}
                });
            } else if (property.size() == 2)
              result[name] = Vec2i(property[0], property[1]);
            else
              result[name] = Vec3i(property[0], property[1], property[2]);
          } else if (isObjectArray(property)) {
            // is a property array
            vector<Properties> local_result;
            std::transform(property.begin(), property.end(),
                std::back_inserter(local_result),
                [&](const json &j) -> Properties {
                  return recursive_construct(j);
                });
            result[name] = local_result;
          } else {
            Exception_("JSON array type not supported");
          }

          break;
        default:
          Exception_("JSON type not matched");

          break;
      }
    }

    return result;
  };

  *this = recursive_construct(j);
};

std::string Properties::toStringInternal(int offset, bool has_first_indent,
    bool has_tail_coma, bool has_tail_newline) const {
  // Approximately json
  const std::string indent(offset, ' ');
  const std::string indent_internal(offset + 2, ' ');
  std::ostringstream out;
  out << format("{}{{\n", has_first_indent ? indent : "");
  for (auto &[name, element] : properties_map) {
    bool is_tail = (&element == &properties_map.rbegin()->second);
    out << format("{}\"{}\": ", indent_internal, name);
    std::visit(
        [offset, &out, indent, indent_internal, is_tail](const auto &x) {
          // Using the more readable variant
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, Properties>) {
            out << x.toStringInternal(offset + 2, false, !is_tail, true);
          } else if constexpr (std::is_same_v<T, vector<Properties>>) {
            out << format("[\n");
            for (auto &p : x)
              out << p.toStringInternal(
                  offset + 4, true, !(&p == &(*x.rbegin())), true);
            out << format("{}]{}\n", indent_internal, is_tail ? "" : ",");
          } else if constexpr (std::is_same_v<T, std::string>)
            out << format("\"{}\"{}\n", x, is_tail ? "" : ",");
          else
            out << format("{}{}\n", x, is_tail ? "" : ",");
        },
        element);
  }

  out << format("{}}}{}{}", indent, has_tail_coma ? "," : "",
      has_tail_newline ? "\n" : "");
  return out.str();
}

RDR_NAMESPACE_END
