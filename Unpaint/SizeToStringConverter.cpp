#include "pch.h"
#include "SizeToStringConverter.h"
#include "SizeToStringConverter.g.cpp"

using namespace std;
using namespace winrt::Windows::Graphics;

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable SizeToStringConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto size = unbox_value_or(value, SizeInt32());
    auto text = format(L"{} x {}", size.Width, size.Height);
    return box_value(hstring(text));
  }

  Windows::Foundation::IInspectable SizeToStringConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
