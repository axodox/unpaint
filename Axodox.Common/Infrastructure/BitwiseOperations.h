#pragma once

namespace Axodox::Infrastructure
{
  template<typename T>
  inline constexpr T bitwise_or(T a, T b)
  {
    return (T)(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
  }

  template<typename T>
  inline constexpr T bitwise_and(const T a, const T b)
  {
    return (T)(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
  }

  template<typename T>
  inline constexpr T bitwise_negate(const T a)
  {
    return (T)(~static_cast<std::underlying_type_t<T>>(a));
  }

  template<typename T>
  inline constexpr bool has_flag(const T a, const T b)
  {
    return bitwise_and(a, b) == b;
  }

  template<typename T>
  inline constexpr bool has_any_flag(const T a, const T b)
  {
    return bitwise_and(a, b) != T(0);
  }

  template<typename T>
  inline void add_flag(T& a, const T b, const bool value = true)
  {
    if (value)
    {
      a = bitwise_or(a, b);
    }
  }

  template<typename T>
  inline void set_flag(T& a, const T b, const bool value)
  {
    if (value)
    {
      a = bitwise_or(a, b);
    }
    else
    {
      a = bitwise_and(a, bitwise_negate(b));
    }
  }

  template <typename T>
  inline void zero_memory(T& value)
  {
    static_assert(std::is_trivially_copyable_v<T>, "Cannot zero out non-trivially copiable types.");

    memset(&value, 0, sizeof(T));
  }

  template <typename T>
  bool are_equal(const T& a, const T& b)
  {
    if constexpr (std::is_trivially_copyable_v<T>)
    {
      return memcmp(&a, &b, sizeof(T)) == 0;
    }
    else
    {
      return a == b;
    }
  }

  template <typename T>
  bool are_equal(std::span<const T> a, std::span<const T> b)
  {
    static_assert(std::is_trivially_copyable<T>::value);

    if (a.size() != b.size()) return false;
    return memcmp(a.data(), b.data(), sizeof(T) * a.size()) == 0;
  }

  template<typename U, typename V, std::enable_if_t<std::conjunction_v<std::is_trivially_copyable<U>, std::is_trivially_copyable<V>>> = true>
  constexpr bool are_equal(const U& a, const V& b)
  {
    static_assert(sizeof(U) == sizeof(V));
    return memcmp(&a, &b, sizeof(U)) == 0;
  }

  bool are_equal(const winrt::Windows::Foundation::IInspectable& a, const winrt::Windows::Foundation::IInspectable& b);

  template <typename T>
  bool is_default(const T& value)
  {
    return are_equal<T>(value, T{});
  }
}