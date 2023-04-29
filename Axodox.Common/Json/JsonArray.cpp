#include "pch.h"
#include "JsonArray.h"

using namespace std;

namespace Axodox::Json
{
  std::unique_ptr<json_value>& json_array::operator[](size_t index)
  {
    return value[index];
  }

  const std::unique_ptr<json_value>& json_array::operator[](size_t index) const
  {
    return value[index];
  }

  void json_array::to_string(std::stringstream& stream) const
  {
    bool isFirst = true;
    stream << "[";
    for (auto& item : value)
    {
      if (isFirst)
      {
        isFirst = false;
      }
      else
      {
        stream << ",";
      }

      if (item)
      {
        item->to_string(stream);
      }
      else
      {
        stream << "null";
      }
    }
    stream << "]";
  }

  std::unique_ptr<json_array> json_array::from_string(std::string_view& text)
  {
    if (text.empty() || text[0] != '[') return nullptr;

    vector<unique_ptr<json_value>> items;
    text = text.substr(1);
    while (true)
    {
      json_skip_whitespace(text);
      if (text.empty()) return nullptr;
      if (text[0] == ']')
      {
        text = text.substr(1);
        return make_unique<json_array>(move(items));
      }

      auto item = json_value::from_string(text);
      if (!item) return nullptr;
      items.push_back(move(item));

      json_skip_whitespace(text);
      if (!text.empty() && text[0] == ',')
      {
        text = text.substr(1);
      }
    }
  }

  json_array::json_array_iterator json_array::begin() const
  {
    return value.begin();
  }

  json_array::json_array_iterator json_array::end() const
  {
    return value.end();
  }
}