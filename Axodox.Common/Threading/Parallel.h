#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  void set_thread_name(const std::wstring& name);
  std::wstring get_thread_name();

  class thread_name_context
  {
  private:
    std::wstring _name;

  public:
    explicit thread_name_context(const std::wstring& name);
    ~thread_name_context();
  };
}