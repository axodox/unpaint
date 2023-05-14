#pragma once
#include "DependencyContainer.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  struct dependency_pointer_type<T, std::enable_if_t<winrt::impl::has_category_v<T>>>
  {
    using type = T;

    inline static T default_value{ nullptr };

    static std::function<T()> get_factory(dependency_container* container)
    {
      return [=] { return create(container); };
    }

    static T create(dependency_container* /*container*/)
    {
      return T{};
    }

    template<typename... TArgs>
    static T make(TArgs&&... args)
    {
      return T{ std::forward<TArgs>(args)... };
    }

    template<typename U>
    static U cast(const T& value)
    {
      return U(value);
    }
  };
}