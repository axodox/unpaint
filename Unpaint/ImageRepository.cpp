#include "pch.h"
#include "ImageRepository.h"
#include "Storage/FileIO.h"

using namespace Axodox::Graphics;
using namespace Axodox::Json;
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

  ImageRepository::~ImageRepository()
  {
    unique_lock lock(_mutex);
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

  std::span<const std::string> ImageRepository::Projects() const
  {
    return _projects;
  }

  std::span<const std::string> ImageRepository::Images() const
  {
    return _images;
  }

  Axodox::Threading::async_action ImageRepository::AddImageAsync(const Axodox::Graphics::TextureData& image, std::optional<int32_t> batchIndex, const ImageMetadata& metadata)
  {
    auto now = zoned_time{ current_zone(), time_point_cast<seconds>(system_clock::now()) };
    auto fileName = format("{:%Y-%m-%d %H-%M-%S}", now);

    string imageMetadata;
    if (batchIndex)
    {
      fileName += std::format(" b{:02}", *batchIndex);
      auto batchMetadata = metadata;
      *batchMetadata.RandomSeed += *batchIndex;
      imageMetadata = stringify_json(batchMetadata);
    }
    else
    {
      imageMetadata = stringify_json(metadata);
    }
    fileName += ".png";

    auto imagePath = GetPath(fileName);

    shared_lock lock(_mutex);
    apartment_context context;
    co_await resume_background();
    auto imageBuffer = image.ToBuffer(imageMetadata);
    auto success = try_write_file(imagePath, imageBuffer);
    co_await context;

    if (success)
    {
      _images.push_back(fileName);
      _events.raise(ImagesChanged, this);
    }
  }

  Axodox::Threading::async_action ImageRepository::AddImageAsync(const Axodox::Graphics::TextureData& image, std::string_view fileName)
  {
    auto imagePath = GetPath(fileName);

    shared_lock lock(_mutex);
    apartment_context context;
    co_await resume_background();
    auto imageBuffer = image.ToBuffer();
    auto success = try_write_file(imagePath, imageBuffer);
    co_await context;

    if (success)
    {
      _images.push_back(string(fileName));
      _events.raise(ImagesChanged, this);
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
    error_code ec;

    _projects.clear();
    auto rootDirectory = ProjectsRoot();
    for (auto& item : filesystem::directory_iterator{ rootDirectory, ec })
    {
      if (!item.is_directory()) continue;

      _projects.push_back(item.path().stem().string());
    }

    _images.clear();
    auto projectDirectory = ImageRoot();
    for (auto& item : filesystem::directory_iterator{ projectDirectory, ec })
    {
      if (item.is_directory() || item.path().extension() != ".png") continue;

      _images.push_back(item.path().filename().string());
    }

    _events.raise(ImagesChanged, this);
  }

  std::filesystem::path ImageRepository::ProjectsRoot(bool isRelative) const
  {
    auto relativePath = "images";
    auto result = isRelative ? relativePath : (filesystem::path{ ApplicationData::Current().LocalCacheFolder().Path().c_str() } / relativePath);
    result.make_preferred();
    return result;
  }

  std::filesystem::path ImageRepository::ImageRoot(bool isRelative) const
  {
    return ProjectsRoot(isRelative) / _projectName;
  }
}