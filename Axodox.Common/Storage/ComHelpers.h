#pragma once
#include "pch.h"

namespace Axodox::Storage
{
  winrt::com_ptr<IStream> to_stream(std::span<const uint8_t> buffer);
}