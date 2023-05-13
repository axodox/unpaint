#include "pch.h"
#include "NanToZeroConverter.h"
#include "NanToZeroConverter.g.cpp"

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable NanToZeroConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto number = unbox_value_or<double>(value, unbox_value_or<float>(value, 0.f));
    return box_value(isnan(number) ? 0.0 : number);
  }

  Windows::Foundation::IInspectable NanToZeroConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
