#include "pch.h"
#include "EqualsConverter.h"
#include "EqualsConverter.g.cpp"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Interop;

namespace winrt::Unpaint::implementation
{
  bool EqualsConverter::IsNegating()
  {
    return _isNegating;
  }

  void EqualsConverter::IsNegating(bool value)
  {
    _isNegating = value;
  }

  Windows::Foundation::IInspectable EqualsConverter::Value()
  {
    return _value;
  }

  void EqualsConverter::Value(Windows::Foundation::IInspectable const& value)
  {
    _value = value;
  }

  Windows::Foundation::IInspectable EqualsConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto areEqual = are_equal(value, _value);
    if (_isNegating) areEqual = !areEqual;

    if (targetType == xaml_typename<IReference<bool>>() || targetType == xaml_typename<bool>())
    {
      return box_value(areEqual);
    }
    else if (targetType == xaml_typename<IReference<Visibility>>() || targetType == xaml_typename<Visibility>())
    {
      return box_value(areEqual ? Visibility::Visible : Visibility::Collapsed);
    }
    else
    {
      return DependencyProperty::UnsetValue();
    }
  }

  Windows::Foundation::IInspectable EqualsConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
