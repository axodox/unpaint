#pragma once
#include "StorageFileToImageSourceConverter.g.h"

namespace winrt::Unpaint::implementation
{
  struct StorageFileToImageSourceConverter : StorageFileToImageSourceConverterT<StorageFileToImageSourceConverter>
  {
    StorageFileToImageSourceConverter() = default;

    Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
  };
}
namespace winrt::Unpaint::factory_implementation
{
  struct StorageFileToImageSourceConverter : StorageFileToImageSourceConverterT<StorageFileToImageSourceConverter, implementation::StorageFileToImageSourceConverter>
  {
  };
}
