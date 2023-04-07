#include "pch.h"
#include "FileIO.h"
#include "FileStream.h"

namespace Axodox::Storage
{
  std::vector<uint8_t> read_file(const std::filesystem::path& path)
  {
    file_stream stream(path, file_mode::read);
    return stream.read_to_end();
  }
  
  void write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer)
  {
    file_stream stream(path, file_mode::write);
    stream.write(buffer);
  }
}