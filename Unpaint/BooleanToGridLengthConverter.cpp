#include "pch.h"
#include "BooleanToGridLengthConverter.h"
#include "BooleanToGridLengthConverter.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  BooleanToGridLengthConverter::BooleanToGridLengthConverter() :
    _trueValue(GridLength{ 1, GridUnitType::Star }),
    _falseValue(GridLength{ 0, GridUnitType::Pixel })
  { }

  Windows::UI::Xaml::GridLength BooleanToGridLengthConverter::TrueValue()
  {
    return _trueValue;
  }

  void BooleanToGridLengthConverter::TrueValue(Windows::UI::Xaml::GridLength value)
  {
    _trueValue = value;
  }

  Windows::UI::Xaml::GridLength BooleanToGridLengthConverter::FalseValue()
  {
    return _falseValue;
  }

  void BooleanToGridLengthConverter::FalseValue(Windows::UI::Xaml::GridLength value)
  {
    _falseValue = value;
  }

  Windows::Foundation::IInspectable BooleanToGridLengthConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return box_value(unbox_value_or(value, false) ? _trueValue : _falseValue);
  }

  Windows::Foundation::IInspectable BooleanToGridLengthConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}