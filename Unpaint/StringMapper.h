#pragma once
#include "pch.h"

namespace winrt::Unpaint
{
  struct StringMapper
  {
    typedef winrt::hstring id_t;
    typedef std::string source_t;
    typedef winrt::hstring target_t;

    id_t get_id(const source_t& sourceItem) const;
    id_t get_id(const target_t& targetItem) const;
    int compare(const target_t& targetItem, const source_t& sourceItem) const;
    target_t map(const source_t& value) const;
  };
}