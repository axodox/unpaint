#include "pch.h"
#include "JsonNumber.h"

using namespace std;

namespace Axodox::Json
{
  void json_number::to_string(std::stringstream& stream) const
  {
    stream << setprecision(numeric_limits<double>::digits10) << value;
  }

  std::unique_ptr<json_number> json_number::from_string(std::string_view& text)
  {
    double number;
    auto result = from_chars(text.data(), text.data() + text.size(), number);
    if (result.ec == errc{})
    {
      text = text.substr(size_t(result.ptr - text.data()));
      return make_unique<json_number>(number);
    }
    else
    {
      return nullptr;
    }
  }
}