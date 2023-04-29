#pragma once
#include "JsonValue.h"

namespace Axodox::Json
{
  struct json_boolean : public json_value_container<bool, json_type::boolean>
  {
    using json_value_container::json_value_container;
    using json_value::to_string;

    virtual void to_string(std::stringstream& stream) const override;
    static std::unique_ptr<json_boolean> from_string(std::string_view& text);
  };

  template <>
  struct json_serializer<bool>
  {
    static std::unique_ptr<json_value> to_json(bool value);
    static bool from_json(const json_value* json, bool& value);
  };
}