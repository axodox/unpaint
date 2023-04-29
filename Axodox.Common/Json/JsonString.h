#pragma once
#include "JsonValue.h"

namespace Axodox::Json
{
  struct json_string : public json_value_container<std::string, json_type::string>
  {
    using json_value_container::json_value_container;
    using json_value::to_string;

    virtual void to_string(std::stringstream& stream) const override;
    static std::unique_ptr<json_string> from_string(std::string_view& text);
  };

  template <>
  struct json_serializer<std::string>
  {
    static std::unique_ptr<json_value> to_json(const std::string& value);
    static bool from_json(const json_value* json, std::string& value);
  };

  template <typename value_t>
  struct json_serializer<value_t, std::enable_if_t<std::is_convertible_v<value_t, std::string>, void>>
  {
    static std::unique_ptr<json_value> to_json(const value_t& value)
    {
      return std::make_unique<json_string>(std::string(value));
    }
  };
}