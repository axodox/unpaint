#include "pch.h"
#include "ImageRepository.h"
#include "Storage/FileIO.h"

using namespace Axodox::Graphics;
using namespace Axodox::Storage;
using namespace std;
using namespace std::chrono;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  ImageRepository::ImageRepository() :
    ImagesChanged(_events)
  { 
    ProjectName("scratch");
  }

  std::string_view ImageRepository::ProjectName()
  {
    return _projectName;
  }

  void ImageRepository::ProjectName(std::string_view value)
  {
    _projectName = value;

    error_code ec;
    filesystem::create_directories(ImageRoot(), ec);

    Refresh();
  }

  std::span<const std::string> ImageRepository::Images() const
  {
    return _images;
  }

  std::string ImageRepository::AddImage(const Axodox::Graphics::TextureData& image)
  {
    auto now = zoned_time{ current_zone(), time_point_cast<seconds>(system_clock::now()) };
    auto fileName = format("{:%Y-%m-%d %H-%M-%S}.png", now);

    auto imagePath = GetPath(fileName);
    auto imageBuffer = image.ToBuffer();
    auto success = try_write_file(imagePath, imageBuffer);

    if (success)
    {
      _images.push_back(fileName);
      _events.raise(ImagesChanged, this);
      return fileName;
    }
    else
    {
      return "";
    }
  }

  bool ImageRepository::RemoveImage(std::string_view imageId)
  {
    auto it = ranges::find(_images, imageId);
    if (it == _images.end()) return false;

    error_code ec;
    auto imagePath = GetPath(imageId);
    if (filesystem::remove(imagePath, ec))
    {
      _images.erase(it);
      _events.raise(ImagesChanged, this);
      return true;
    }
    else
    {
      return false;
    }
  }

  Axodox::Graphics::TextureData ImageRepository::GetImage(std::string_view imageId) const
  {
    auto imagePath = GetPath(imageId);
    auto imageBuffer = try_read_file(imagePath);

    if (!imageBuffer.empty())
    {
      return TextureData::FromBuffer(imageBuffer);
    }
    else
    {
      return {};
    }
  }

  std::filesystem::path ImageRepository::GetPath(std::string_view imageId, bool isRelative) const
  {
    return ImageRoot(isRelative) / imageId;
  }

  void ImageRepository::Refresh()
  {
    _images.clear();

    error_code ec;
    auto directory = ImageRoot();
    for (auto& item : filesystem::directory_iterator{ directory, ec })
    {
      if (item.is_directory() || item.path().extension() != ".png") continue;

      _images.push_back(item.path().filename().string());
    }

    _events.raise(ImagesChanged, this);
  }

  std::filesystem::path ImageRepository::ImageRoot(bool isRelative) const
  {
    auto relativePath = "images/" + _projectName;
    auto result = isRelative ? relativePath : (filesystem::path{ ApplicationData::Current().LocalCacheFolder().Path().c_str() } / relativePath);
    result.make_preferred();
    return result;
  }
}