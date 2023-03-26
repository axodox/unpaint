#include "pch.h"
#include "ComHelpers.h"

using namespace winrt;

namespace Axodox::Storage
{
  winrt::com_ptr<IStream> to_stream(std::span<const uint8_t> buffer)
  {
    auto data = GlobalAlloc(0, buffer.size());
    memcpy(data, buffer.data(), buffer.size());

    com_ptr<IStream> result;
    check_hresult(CreateStreamOnHGlobal(data, true, result.put()));
    return result;
  }
}