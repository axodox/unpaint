#pragma once
#include "pch.h"

namespace winrt::Unpaint
{
  class OptionPropertyBase
  {
  public:
    virtual ~OptionPropertyBase() = default;
  };

  template<typename T>
  class OptionProperty : public OptionPropertyBase
  {
  private:
    Axodox::Infrastructure::event_owner _events;
    T _value;

  public:
    Axodox::Infrastructure::event_publisher<OptionPropertyBase*> ValueChanged;
    
    OptionProperty(const T value = {}) :
      _value(value),
      ValueChanged(_events)
    { }

    operator const T& () const
    {
      return _value;
    }

    void operator=(const T& value)
    {
      if (value == _value) return;

      _value = value;
      _events.raise(ValueChanged, this);
    }

    const T* operator->() const
    {
      return &_value;
    }

    const T& operator*() const
    {
      return _value;
    }
  };
}