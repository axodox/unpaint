#pragma once
#include "Infrastructure/Traits.h"

namespace Axodox::Storage
{
  class stream
  {
  public:
    virtual void write(std::span<const uint8_t> buffer) = 0;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) = 0;

    virtual size_t position() const = 0;
    virtual void seek(size_t position) = 0;

    virtual size_t length() const = 0;
    virtual ~stream() = default;

    template<Infrastructure::trivially_copyable T>
    void read(T& value)
    {
      read(std::span<uint8_t>{ reinterpret_cast<uint8_t*>(&value), sizeof(T) });
    }

    template<Infrastructure::trivially_copyable T>
    void write(const T& value)
    {
      write(std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>(&value), sizeof(T) });
    }

    template<typename T>
    void read(std::span<T> value)
    {
      read(std::span<uint8_t>{ reinterpret_cast<uint8_t*>(value.data()), value.size_bytes() });
    }

    template<typename T>
    void write(std::span<const T> value)
    {
      write(std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>(value.data()), value.size_bytes() });
    }

    std::string read_line();

    std::vector<uint8_t> read_to_end();
  };
}