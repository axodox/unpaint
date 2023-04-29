#include "pch.h"
#include "JsonBoolean.h"

using namespace std;

namespace Axodox::Json
{
  void json_boolean::to_string(std::stringstream& stream) const
  {
    stream << (value ? "true" : "false");
  }

  std::unique_ptr<json_boolean> json_boolean::from_string(std::string_view& text)
  {
    if (text.compare(0, 4, "true") == 0)
    {
      text = text.substr(4);
      return make_unique<json_boolean>(true);
    }
    else if (text.compare(0, 5, "false") == 0)
    {
      text = text.substr(5);
      return make_unique<json_boolean>(false);
    }
    else
    {
      return nullptr;
    }
  }

  std::unique_ptr<json_value> json_serializer<bool>::to_json(bool value)
  {
    return make_unique<json_boolean>(value);
  }

  bool json_serializer<bool>::from_json(const json_value* json, bool& value)
  {
    if (json && json->type() == json_type::boolean)
    {
      value = static_cast<const json_boolean*>(json)->value;
      return true;
    }
    else
    {
      return false;
    }
  }
}