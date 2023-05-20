#pragma once
#include "EqualsConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct EqualsConverter : EqualsConverterT<EqualsConverter>
  {
    EqualsConverter() = default;

    bool IsNegating();
    void IsNegating(bool value);

    Windows::Foundation::IInspectable Value();
    void Value(Windows::Foundation::IInspectable const& value);

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);

  private:
    bool _isNegating = false;
    Windows::Foundation::IInspectable _value;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct EqualsConverter : EqualsConverterT<EqualsConverter, implementation::EqualsConverter>
  {
  };
}
