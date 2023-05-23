#include "pch.h"
#include "StorageFileToImageSourceConverter.h"
#include "StorageFileToImageSourceConverter.g.cpp"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;

namespace winrt::Unpaint::implementation
{
  Windows::Foundation::IInspectable StorageFileToImageSourceConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto imageFile = value.try_as<StorageFile>();
    if (!imageFile) return nullptr;

    BitmapImage result;
    result.UriSource(Uri(imageFile.Path()));

    return result;
  }

  Windows::Foundation::IInspectable StorageFileToImageSourceConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
