#include "pch.h"
#include "IsNanToBooleanConverter.h"
#include "IsNanToBooleanConverter.g.cpp"

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable IsNanToBooleanConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    return box_value(isnan(unbox_value_or<double>(value, unbox_value_or<float>(value, 0.f))));
  }

  Windows::Foundation::IInspectable IsNanToBooleanConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
