#pragma once
#include "BooleanToGridLengthConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct BooleanToGridLengthConverter : BooleanToGridLengthConverterT<BooleanToGridLengthConverter>
  {
    BooleanToGridLengthConverter();

    Windows::UI::Xaml::GridLength TrueValue();
    void TrueValue(Windows::UI::Xaml::GridLength value);

    Windows::UI::Xaml::GridLength FalseValue();
    void FalseValue(Windows::UI::Xaml::GridLength value);

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);

  private:
    Windows::UI::Xaml::GridLength _trueValue, _falseValue;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct BooleanToGridLengthConverter : BooleanToGridLengthConverterT<BooleanToGridLengthConverter, implementation::BooleanToGridLengthConverter>
  {
  };
}
