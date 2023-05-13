#pragma once
#include "IsNanToBooleanConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct IsNanToBooleanConverter : IsNanToBooleanConverterT<IsNanToBooleanConverter>
  {
    IsNanToBooleanConverter() = default;

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct IsNanToBooleanConverter : IsNanToBooleanConverterT<IsNanToBooleanConverter, implementation::IsNanToBooleanConverter>
  {
  };
}
