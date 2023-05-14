#include "pch.h"
#include "StringMapper.h"

namespace winrt::Unpaint
{
  StringMapper::id_t StringMapper::get_id(const source_t& sourceItem) const
  {
    return winrt::to_hstring(sourceItem);
  }

  StringMapper::id_t StringMapper::get_id(const target_t& targetItem) const
  {
    return targetItem;
  }

  int StringMapper::compare(const target_t& targetItem, const source_t& sourceItem) const
  {
    return strcmp(winrt::to_string(targetItem).c_str(), sourceItem.c_str());
  }

  StringMapper::target_t StringMapper::map(const source_t& value) const
  {
    return winrt::to_hstring(value);
  }
}