#include "pch.h"
#include "ArrayStream.h"

using namespace std;

namespace Axodox::Storage
{
  array_stream::array_stream(std::span<const uint8_t> buffer) :
    _buffer({ const_cast<uint8_t*>(buffer.data()), buffer.size() }),
    _isReadOnly(true),
    _position(0u)
  { }

  array_stream::array_stream(std::span<uint8_t> buffer) :
    _buffer(buffer),
    _isReadOnly(false),
    _position(0u)
  { }

  void array_stream::write(std::span<const uint8_t> buffer)
  {
    if (_isReadOnly)
    {
      throw logic_error("Cannot write to a read-only array!");
    }

    if (_position + buffer.size() > _buffer.size())
    {
      throw logic_error("Cannot write past the end of the array!");
    }

    memcpy(current(), buffer.data(), buffer.size());
    _position += buffer.size();
  }

  size_t array_stream::read(std::span<uint8_t> buffer, bool partial)
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

    memcpy(buffer.data(), current(), length);
    _position += buffer.size();

    return length;
  }

  size_t array_stream::position() const
  {
    return _position;
  }

  void array_stream::seek(size_t position)
  {
    if (_position > _buffer.size())
    {
      throw logic_error("Cannot seek past the end of the array!");
    }

    _position = position;
  }

  size_t array_stream::length() const
  {
    return _buffer.size();
  }
  
  array_stream::operator std::span<const uint8_t>() const
  {
    return _buffer;
  }

  uint8_t* array_stream::current()
  {
    return _buffer.data() + _position;
  }
}