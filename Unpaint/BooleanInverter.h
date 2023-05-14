#pragma once
#include "BooleanInverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct BooleanInverter : BooleanInverterT<BooleanInverter>
  {
    BooleanInverter() = default;

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct BooleanInverter : BooleanInverterT<BooleanInverter, implementation::BooleanInverter>
  {
  };
}
