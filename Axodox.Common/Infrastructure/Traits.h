#pragma once
#include "pch.h"

namespace Axodox::Infrastructure
{
  template<template<typename...> class, typename...>
  struct is_instantiation_of : public std::false_type {};

  template<template<typename...> class U, typename... T>
  struct is_instantiation_of<U, U<T...>> : public std::true_type {};

  template<typename T>
  concept trivially_copyable = std::is_trivially_copyable_v<T>;

  struct supports_new_test
  {
    template<typename T>
    static std::true_type test(decltype(new T())*);

    template<typename T>
    static std::false_type test(...);
  };

  template<class T>
  struct supports_new : decltype(supports_new_test::test<T>(nullptr))
  { };
}