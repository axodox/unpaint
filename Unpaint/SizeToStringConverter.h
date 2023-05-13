#pragma once
#include "SizeToStringConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct SizeToStringConverter : SizeToStringConverterT<SizeToStringConverter>
  {
    SizeToStringConverter() = default;

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SizeToStringConverter : SizeToStringConverterT<SizeToStringConverter, implementation::SizeToStringConverter>
  {
  };
}
