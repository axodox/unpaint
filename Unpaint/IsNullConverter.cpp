#include "pch.h"
#include "IsNullConverter.h"
#include "IsNullConverter.g.cpp"

using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable IsNullConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    if (targetType == xaml_typename<bool>())
    {
      return box_value(value == nullptr);
    }

    if (targetType == xaml_typename<Visibility>())
    {
      return box_value(value == nullptr ? Visibility::Visible : Visibility::Collapsed);
    }

    return nullptr;
  }

  Windows::Foundation::IInspectable IsNullConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
