#include "pch.h"
#include "BooleanInverter.h"
#include "BooleanInverter.g.cpp"

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable BooleanInverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return box_value(!unbox_value_or(value, false));
  }

  winrt::Windows::Foundation::IInspectable BooleanInverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return box_value(!unbox_value_or(value, false));
  }
}
