#pragma once
#include "pch.h"

namespace Axodox::Storage
{
  std::vector<uint8_t> read_file(const std::filesystem::path& path);
}