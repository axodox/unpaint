#include "pch.h"
#include "Win32.h"

namespace Axodox::Infrastructure
{
  std::wstring to_wstring(std::string_view text)
  {
    const auto expectedSize = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), nullptr, 0);

    if (expectedSize == 0)
    {
      return {};
    }

    std::wstring result(expectedSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), result.data(), expectedSize);
    return result;
  }
}
