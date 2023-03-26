#pragma once
#include "Stream.h"

namespace Axodox::Storage
{
  enum class file_mode : uint8_t
  {
    none = 0,
    read = 1,
    write = 2,
    read_write = read | write
  };

  class file_stream : public stream
  {
  public:
    file_stream(const std::filesystem::path& path, file_mode mode);

    file_stream(const file_stream&) = delete;
    file_stream& operator=(const file_stream&) = delete;

    file_stream(file_stream&& other);
    file_stream& operator=(file_stream&& other);

    virtual void write(std::span<const uint8_t> buffer) override;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) override;

    virtual size_t position() const override;
    virtual void seek(size_t position) override;
    virtual size_t length() const override;

    const std::filesystem::path& path() const;
    void flush();

    using stream::write;
    using stream::read;

  private:
    std::filesystem::path _path;
    file_mode _mode;
    FILE* _file;

    void check_mode(file_mode mode) const;
  };
}