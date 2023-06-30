#include "pch.h"
#include "BooleanInverter.h"
#include "BooleanInverter.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable BooleanInverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto result = !unbox_value_or(value, false);

    if (targetType == xaml_typename<Visibility>())
    {
      return box_value(result ? Visibility::Visible : Visibility::Collapsed);
    }
    else
    {
      return box_value(result);
    }
  }

  winrt::Windows::Foundation::IInspectable BooleanInverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return box_value(!unbox_value_or(value, false));
  }
}
