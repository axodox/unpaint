#include "pch.h"
#include "Stream.h"

using namespace std;

namespace Axodox::Storage
{
  std::string stream::read_line()
  {
    string buffer;
    buffer.resize(1024);

    string result;
    for (;;)
    {
      auto length = read(span<uint8_t>{ reinterpret_cast<uint8_t*>(buffer.data()), buffer.size() }, true);
      if (length == 0) return result;

      auto newLine = find(buffer.data(), buffer.data() + length, '\n');
      result += string_view(buffer.data(), newLine - buffer.data());

      if (*newLine == '\n')
      {
        seek(position() - length + (newLine - buffer.data()) + 1);
        return result;
      }
    }
  }

  std::vector<uint8_t> stream::read_to_end()
  {
    vector<uint8_t> buffer;
    buffer.resize(length());

    read(buffer);
    return buffer;
  }
}