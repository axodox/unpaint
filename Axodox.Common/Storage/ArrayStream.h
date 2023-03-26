#pragma once
#include "Stream.h"

namespace Axodox::Storage
{
  class array_stream : public stream
  {
  public:
    explicit array_stream(std::span<const uint8_t> buffer);
    explicit array_stream(std::span<uint8_t> buffer);

    virtual void write(std::span<const uint8_t> buffer) override;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) override;

    virtual size_t position() const override;
    virtual void seek(size_t position) override;
    virtual size_t length() const override;

    using stream::write;
    using stream::read;

    operator std::span<const uint8_t>() const;

  private:
    std::span<uint8_t> _buffer;
    bool _isReadOnly;
    size_t _position;

    uint8_t* current();
  };
}