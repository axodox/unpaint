#include "pch.h"
#include "StringToImageSourceConverter.h"
#include "StringToImageSourceConverter.g.cpp"
#include "Infrastructure/DependencyContainer.h"
#include "ImageRepository.h"

using namespace Axodox::Infrastructure;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;

namespace winrt::Unpaint::implementation
{
  const size_t StringToImageSourceConverter::_cachedImageCount = 16;
  std::list<std::pair<hstring, Windows::UI::Xaml::Media::Imaging::BitmapImage>> StringToImageSourceConverter::_cachedImages;

  Windows::Foundation::IInspectable StringToImageSourceConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    auto imageId = unbox_value_or(value, L"");
    if (imageId.empty()) return nullptr;

    for (auto& [id, image] : _cachedImages)
    {
      if (id == imageId) return image;
    }
    
    auto imageRepository = dependencies.resolve<ImageRepository>();
    auto imagePath = imageRepository->GetPath(to_string(imageId));
    if (imagePath.empty()) return nullptr;
        
    BitmapImage result;
    result.UriSource(Uri(imagePath.c_str()));

    _cachedImages.push_front({ imageId, result });
    while (_cachedImages.size() > _cachedImageCount) _cachedImages.pop_back();

    return result;
  }

  Windows::Foundation::IInspectable StringToImageSourceConverter::ConvertBack(Windows::Foundation::IInspectable const& /*value*/, Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
  {
    throw hresult_not_implemented();
  }
}
