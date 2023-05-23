#pragma once
#include "BooleanSwitchConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct BooleanSwitchConverter : BooleanSwitchConverterT<BooleanSwitchConverter>
  {
    BooleanSwitchConverter() = default;

    Windows::Foundation::IInspectable TrueValue();
    void TrueValue(Windows::Foundation::IInspectable const& value);

    Windows::Foundation::IInspectable FalseValue();
    void FalseValue(Windows::Foundation::IInspectable const& value);

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);

  private:
    Windows::Foundation::IInspectable _trueValue, _falseValue;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct BooleanSwitchConverter : BooleanSwitchConverterT<BooleanSwitchConverter, implementation::BooleanSwitchConverter>
  {
  };
}
