#pragma once
#include "pch.h"

namespace Axodox::Storage
{
  std::filesystem::path app_folder();

  std::vector<uint8_t> read_file(const std::filesystem::path& path);
  std::vector<uint8_t> try_read_file(const std::filesystem::path& path);

  void write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);
  bool try_write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer);

  std::optional<std::string> try_read_text(const std::filesystem::path& path);
  bool try_write_text(const std::filesystem::path& path, std::string_view text);
}