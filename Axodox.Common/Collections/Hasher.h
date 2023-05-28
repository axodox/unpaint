#pragma once
#include "pch.h"

namespace Axodox::Collections
{
  template <typename TValue>
  struct trivial_hasher
  {
    size_t operator()(const TValue& value) const
    {
      static_assert(std::is_trivially_copyable<TValue>::value, "The type must be trivially copyable for trivial hasher.");

      //If the value type is smaller than size_t, return the value itself (with padding if needed)
      if constexpr (sizeof(TValue) <= sizeof(size_t))
      {
        size_t result = {};
        memcpy(&result, &value, sizeof(TValue));
        return result;
      }
      //Otherwise call efficient MSVC FNV hash implementation
      else
      {
        return std::_Fnv1a_append_bytes(std::_FNV_offset_basis, reinterpret_cast<const unsigned char*>(&value), sizeof(TValue));
      }
    }
  };

  template <typename TValue>
  struct trivial_comparer
  {
    bool operator()(const TValue& a, const TValue& b) const
    {
      static_assert(std::is_trivially_copyable<TValue>::value, "The type must be trivially copyable for trivial comparer.");
      return memcmp(&a, &b, sizeof(TValue)) == 0;
    }
  };

  template <typename TKey, typename TValue>
  using trivial_map = std::unordered_map<TKey, TValue, trivial_hasher<TKey>, trivial_comparer<TKey>>;
}