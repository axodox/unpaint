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
}