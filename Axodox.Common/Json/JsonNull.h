#pragma once
#include "JsonValue.h"

namespace Axodox::Json
{
  struct json_null : public json_value
  {
    using json_value::to_string;

    virtual json_type type() const override;

    virtual void to_string(std::stringstream& stream) const override;
    static std::unique_ptr<json_null> from_string(std::string_view& text);
  };
}