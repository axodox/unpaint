#include "pch.h"
#include "FileStream.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Storage
{
  file_stream::file_stream(const std::filesystem::path& path, file_mode mode) :
    _path(path),
    _mode(mode)
  {
    const wchar_t* fileMode;
    switch (mode)
    {
    case file_mode::read:
      fileMode = L"rb";
      break;
    case file_mode::write:
      fileMode = L"wb";
      break;
    case file_mode::read_write:
      fileMode = L"wb+";
      break;
    default:
      throw logic_error("Invalid file mode specified!");
    }

    if (_wfopen_s(&_file, path.c_str(), fileMode) != 0)
    {
      throw runtime_error("Failed to open file!");
    }
  }

  file_stream::~file_stream()
  {
    fclose(_file);
  }

  file_stream::file_stream(file_stream&& other)
  {
    *this = move(other);
  }

  file_stream& file_stream::operator=(file_stream&& other)
  {
    _file = other._file;
    other._file = nullptr;

    _mode = other._mode;
    other._mode = file_mode::none;

    _path = other._path;
    other._path = L"";

    return *this;
  }

  void file_stream::write(std::span<const uint8_t> buffer)
  {
    check_mode(file_mode::write);
    fwrite(buffer.data(), 1, buffer.size(), _file);
  }

  size_t file_stream::read(std::span<uint8_t> buffer, bool partial)
  {
    check_mode(file_mode::read);
    auto length = fread(buffer.data(), 1, buffer.size(), _file);

    if (length < buffer.size() && !partial)
    {
      throw logic_error("Cannot read past the end of the file!");
    }

    return length;
  }

  size_t file_stream::position() const
  {
    check_mode(file_mode::read_write);
    return ftell(_file);
  }

  void file_stream::seek(size_t position)
  {
    check_mode(file_mode::read_write);
    fseek(_file, long(position), SEEK_SET);
  }

  size_t file_stream::length() const
  {
    check_mode(file_mode::read_write);

    auto currentPosition = position();

    fseek(_file, 0, SEEK_END);
    auto length = position();

    fseek(_file, long(currentPosition), SEEK_SET);
    return length;
  }

  const std::filesystem::path& file_stream::path() const
  {
    return _path;
  }

  void file_stream::flush()
  {
    fflush(_file);
  }

  void file_stream::check_mode(file_mode mode) const
  {
    if (!has_any_flag(_mode, mode))
    {
      throw logic_error("Attempted invalid operation on file stream!");
    }
  }
}