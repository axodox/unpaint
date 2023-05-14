#pragma once
#include "Graphics/Textures/TextureData.h"
#include "Infrastructure/Events.h"

namespace winrt::Unpaint
{
  class ImageRepository
  {
    Axodox::Infrastructure::event_owner _events;

  public:
    ImageRepository();

    std::string_view ProjectName();
    void ProjectName(std::string_view value);

    std::span<const std::string> Images() const;
    Axodox::Infrastructure::event_publisher<ImageRepository*> ImagesChanged;

    std::string AddImage(const Axodox::Graphics::TextureData& image);
    bool RemoveImage(std::string_view imageId);
    Axodox::Graphics::TextureData GetImage(std::string_view imageId) const;
    std::filesystem::path GetPath(std::string_view imageId) const;

    void Refresh();

  private:
    std::string _projectName;
    std::vector<std::string> _images;

    std::filesystem::path ImageRoot() const;
  };
}