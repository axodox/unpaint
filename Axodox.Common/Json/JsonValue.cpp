#include "pch.h"
#include "JsonValue.h"
#include "JsonNull.h"
#include "JsonBoolean.h"
#include "JsonNumber.h"
#include "JsonString.h"
#include "JsonArray.h"
#include "JsonObject.h"

using namespace std;

namespace Axodox::Json
{
  std::string json_value::to_string() const
  {
    stringstream stream;
    to_string(stream);
    return stream.str();
  }

  std::unique_ptr<json_value> json_value::from_string(std::string_view& text)
  {
    json_skip_whitespace(text);

    if (text.empty()) return nullptr;

    switch (text.front())
    {
    case 'n':
      return json_null::from_string(text);
    case 't':
    case 'f':
      return json_boolean::from_string(text);
    case '"':
      return json_string::from_string(text);
    case '[':
      return json_array::from_string(text);
    case '{':
      return json_object::from_string(text);
    default:
      return json_number::from_string(text);
    }
  }

  void json_skip_whitespace(std::string_view& text)
  {
    for (auto& character : text)
    {
      switch (character)
      {
      case ' ':
      case '\r':
      case '\n':
      case '\t':
        //Skip white space
        break;
      default:
        text = text.substr(size_t(&character - text.data()));
        return;
      }
    }
  }
}