#pragma once
#include "IsNullConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct IsNullConverter : IsNullConverterT<IsNullConverter>
  {
    IsNullConverter() = default;

    bool IsNegating();
    void IsNegating(bool value);

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);

  private:
    bool _isNegating = false;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct IsNullConverter : IsNullConverterT<IsNullConverter, implementation::IsNullConverter>
  {
  };
}
