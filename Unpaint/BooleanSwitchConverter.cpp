#include "pch.h"
#include "BooleanSwitchConverter.h"
#include "BooleanSwitchConverter.g.cpp"

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable BooleanSwitchConverter::TrueValue()
  {
    return _trueValue;
  }

  void BooleanSwitchConverter::TrueValue(Windows::Foundation::IInspectable const& value)
  {
    _trueValue = value;
  }

  Windows::Foundation::IInspectable BooleanSwitchConverter::FalseValue()
  {
    return _falseValue;
  }

  void BooleanSwitchConverter::FalseValue(Windows::Foundation::IInspectable const& value)
  {
    _falseValue = value;
  }

  Windows::Foundation::IInspectable BooleanSwitchConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return unbox_value_or(value, false) ? _trueValue : _falseValue;
  }

  Windows::Foundation::IInspectable BooleanSwitchConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
