#include "pch.h"
#include "FileIO.h"
#include "FileStream.h"

using namespace winrt;
using namespace std;
using namespace std::filesystem;

namespace Axodox::Storage
{
  std::filesystem::path app_folder()
  {
    wstring filePath(MAX_PATH, L'\0');
    GetModuleFileName(nullptr, filePath.data(), (DWORD)filePath.size());
    return filesystem::path(filePath).parent_path();
  }

  std::vector<uint8_t> read_file(const std::filesystem::path& path)
  {
    file_stream stream(path, file_mode::read);
    return stream.read_to_end();
  }

  std::vector<uint8_t> try_read_file(const std::filesystem::path& path)
  {
    try
    {
      return read_file(path);
    }
    catch(...)
    {
      return {};
    }
  }

  void write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer)
  {
    file_stream stream(path, file_mode::write);
    stream.write(buffer);
  }

  bool try_write_file(const std::filesystem::path& path, std::span<const uint8_t> buffer)
  {
    try
    {
      write_file(path, buffer);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  enum class text_encoding : uint32_t
  {
    utf8,
    utf16le
  };

  text_encoding get_text_encoding(file_stream& stream)
  {
    //Save position
    auto position = stream.position();

    //Try read BOM - this read can be partial, but that is fine for us
    uint32_t bom;
    stream.read(bom);

    //Compare with BOM values - we assume little endian encoding which is true on our target platforms
    if ((bom & 0xFFFFFF) == 0xBFBBEF)
    {
      stream.seek(position + 3);
      return text_encoding::utf8;
    }
    else if ((bom & 0xFFFF) == 0xFEFF)
    {
      stream.seek(position + 2);
      return text_encoding::utf16le;
    }
    else
    {
      stream.seek(position);
    }

    //Default to UTF8
    return text_encoding::utf8;
  }

  std::optional<std::string> try_read_text(const std::filesystem::path& path)
  {
    try
    {
      file_stream stream{ path, file_mode::read };
      auto encoding = get_text_encoding(stream);

      vector<uint8_t> buffer(stream.length() - stream.position());
      stream.read(buffer);

      switch (encoding)
      {
      case text_encoding::utf8:
        return string{ reinterpret_cast<char*>(buffer.data()), buffer.size() };
      case text_encoding::utf16le:
        return to_string(wstring{ reinterpret_cast<wchar_t*>(buffer.data()), buffer.size() / 2 });
      default:
        return nullopt;
      }
    }
    catch (...)
    {
      return nullopt;
    }
  }

  bool try_write_text(const std::filesystem::path& path, std::string_view text)
  {
    try
    {
      file_stream stream{ path, file_mode::write };

      const auto bom = 0xBFBBEF;
      stream.write({ reinterpret_cast<const uint8_t*>(&bom), 3});
      stream.write(text);

      return true;
    }
    catch (...)
    {
      return false;
    }
  }
}