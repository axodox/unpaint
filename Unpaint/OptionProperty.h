#pragma once
#include "pch.h"
#include "Storage/SettingManager.h"

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

  template<typename T>
  class PersistentOptionProperty : public OptionProperty<T>
  {
  private:
    const char* _name;

    static std::shared_ptr<Axodox::Storage::SettingManager> SettingManager()
    {
      return Axodox::Infrastructure::dependencies.resolve<Axodox::Storage::SettingManager>();
    }

    void OnValueChanged(OptionPropertyBase*)
    {
      SettingManager()->StoreSetting(_name, **this);
    }

  public:
    PersistentOptionProperty(const char* name, const T value = {}) :
      OptionProperty<T>(SettingManager()->LoadSettingOr(name, value)),
      _name(name)
    { 
      OptionProperty<T>::ValueChanged(Axodox::Infrastructure::no_revoke, Axodox::Infrastructure::event_handler{ this, &PersistentOptionProperty<T>::OnValueChanged });
    }    

    using OptionProperty<T>::operator const T&;
    using OptionProperty<T>::operator*;
    using OptionProperty<T>::operator->;
    using OptionProperty<T>::operator=;
  };
}