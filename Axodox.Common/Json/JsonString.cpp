#include "pch.h"
#include "JsonString.h"

namespace Axodox::Json
{
  void json_string::to_string(std::stringstream& stream) const
  {
    stream << "\"";
    for (auto character : value)
    {
      switch (character)
      {
      case '"':
        stream << "\\\"";
        break;
      case '\r':
        stream << "\\r";
        break;
      case '\n':
        stream << "\\n";
        break;
      case '\t':
        stream << "\\t";
        break;
      case '\0':
        stream << "\"";
        return;
      default:
        stream << character;
        break;
      }
    }

    stream << "\"";
  }

  std::unique_ptr<json_string> json_string::from_string(std::string_view& text)
  {
    if (text.empty()) return nullptr;

    std::stringstream result;
    bool isFirst = true;
    bool isEscaping = false;
    const char* end = nullptr;
    for (auto& character : text)
    {
      if (isFirst)
      {
        if (character != '"') return nullptr;
        isFirst = false;
      }
      else if (isEscaping)
      {
        isEscaping = false;
        switch (character)
        {
        case '0':
          result << '\0';
          break;
        case 'r':
          result << '\r';
          break;
        case 'n':
          result << '\n';
          break;
        case 't':
          result << '\t';
          break;
        case '"':
          result << '"';
          break;
        default:
          result << character;
          break;
        }
      }
      else if (character == '\\')
      {
        isEscaping = true;
      }
      else if (character == '"')
      {
        end = &character;
        break;
      }
      else
      {
        result << character;
      }
    }

    if (!end) return nullptr;
    text = text.substr(size_t(end - text.data()) + 1);

    return make_unique<json_string>(result.str());
  }

  std::unique_ptr<json_value> json_serializer<std::string>::to_json(const std::string& value)
  {
    return make_unique<json_string>(value);
  }

  bool json_serializer<std::string>::from_json(const json_value* json, std::string& value)
  {
    if (json && json->type() == json_type::string)
    {
      value = static_cast<const json_string*>(json)->value;
      return true;
    }
    else
    {
      return false;
    }
  }
}