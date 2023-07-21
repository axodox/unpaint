#include "pch.h"
#include "IsNullConverter.h"
#include "IsNullConverter.g.cpp"

using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  bool IsNullConverter::IsNegating()
  {
    return _isNegating;
  }

  void IsNullConverter::IsNegating(bool value)
  {
    _isNegating = value;
  }

  Windows::Foundation::IInspectable IsNullConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto result = value == nullptr;
    if (_isNegating) result = !result;

    if (targetType == xaml_typename<bool>())
    {
      return box_value(result);
    }

    if (targetType == xaml_typename<Visibility>())
    {
      return box_value(result ? Visibility::Visible : Visibility::Collapsed);
    }

    return nullptr;
  }

  Windows::Foundation::IInspectable IsNullConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
