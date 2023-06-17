#pragma once
#include "pch.h"
#include "Json/JsonSerializer.h"
#include "Threading/AsyncOperation.h"
#include "winrt/Unpaint.h"

namespace winrt::Unpaint
{
  struct ModelMetadata : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<std::string> Id;
    Axodox::Json::json_property<std::string> Name;
    Axodox::Json::json_property<std::string> Website;
    Axodox::Json::json_property<std::string> AccessToken;

    ModelMetadata();
  };

  struct ModelInfo
  {
    std::string Id;
    std::string Name;
    std::string Website;
    std::string AccessToken;

    auto operator<=>(const ModelInfo&) const = default;
    bool operator<(const ModelInfo&) const = default;

    operator ModelViewModel() const;
  };

  class ModelRepository
  {
  public:
    ModelRepository();

    bool TryInstallHuggingFaceModel(std::string_view modelId, Axodox::Threading::async_operation& operation);
    bool AddModelFromDisk(const winrt::Windows::Storage::StorageFolder& folder);
    void UninstallModel(std::string_view modelId);

    std::filesystem::path Root() const;
    std::set<ModelInfo> Models() const;
    void Refresh();

    std::optional<ModelInfo> GetModel(std::string_view modelId) const;

    Windows::Foundation::IAsyncOperation<Windows::Storage::StorageFolder> GetModelFolderAsync(std::string_view modelId) const;
    std::unordered_map<std::string, Windows::Storage::StorageFile> GetModelFiles(std::string_view modelId) const;

  private:
    mutable std::mutex _mutex;
    std::filesystem::path _root;
    std::set<ModelInfo> _models;
  };
}