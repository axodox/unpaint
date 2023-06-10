#pragma once
#include "StringToImageSourceConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct StringToImageSourceConverter : StringToImageSourceConverterT<StringToImageSourceConverter>
  {
    StringToImageSourceConverter() = default;

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);

  private:
    static const size_t _cachedImageCount;
    static std::list<std::pair<hstring, Windows::UI::Xaml::Media::Imaging::BitmapImage>> _cachedImages;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct StringToImageSourceConverter : StringToImageSourceConverterT<StringToImageSourceConverter, implementation::StringToImageSourceConverter>
  {
  };
}
