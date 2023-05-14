#include "pch.h"
#include "MemoryStream.h"

using namespace std;

namespace Axodox::Storage
{
  void memory_stream::write(std::span<const uint8_t> buffer)
  {
    auto newSize = _position + buffer.size();
    if (newSize > _buffer.size())
    {
      _buffer.resize(newSize);
    }

    memcpy(current(), buffer.data(), buffer.size());
    _position += buffer.size();
  }

  size_t memory_stream::read(std::span<uint8_t> buffer, bool partial)
  {
    auto length = buffer.size();

    if (partial)
    {
      length = min(_buffer.size() - _position, buffer.size());
    }
    else
    {
      if (_position + buffer.size() > _buffer.size())
      {
        throw logic_error("Cannot read past the end of the array!");
      }
    }

    memcpy(buffer.data(), current(), buffer.size());
    _position += buffer.size();

    return length;
  }

  size_t memory_stream::position() const
  {
    return _position;
  }

  void memory_stream::seek(size_t position)
  {
    if (_position > _buffer.size())
    {
      throw logic_error("Cannot seek past the end of the array!");
    }

    _position = position;
  }

  size_t memory_stream::length() const
  {
    return _buffer.size();;
  }

  uint8_t* memory_stream::current()
  {
    return _buffer.data() + _position;
  }

  memory_stream::operator std::span<uint8_t>()
  {
    return _buffer;
  }

  memory_stream::operator std::span<const uint8_t>() const
  {
    return _buffer;
  }

  uint8_t* memory_stream::data()
  {
    return _buffer.data();
  }

  const uint8_t* memory_stream::data() const
  {
    return _buffer.data();
  }
}