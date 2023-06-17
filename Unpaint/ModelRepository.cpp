#include "pch.h"
#include "ModelRepository.h"
#include "Infrastructure/DependencyContainer.h"
#include "Infrastructure/Text.h"
#include "Storage/FileIO.h"
#include "Storage/UwpStorage.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace Axodox::Web;
using namespace std;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::AccessCache;

namespace winrt::Unpaint
{
  ModelMetadata::ModelMetadata() :
    Id(this, "id"),
    Name(this, "name"),
    AccessToken(this, "accessToken")
  { }

  ModelRepository::ModelRepository() :
    _root((ApplicationData::Current().LocalCacheFolder().Path() + L"\\models").c_str())
  { 
    //Ensure models directory
    if (!filesystem::exists(_root))
    {
      filesystem::create_directories(_root);
    }

    //Load model list
    Refresh();
  }

  bool ModelRepository::TryInstallHuggingFaceModel(std::string_view modelId, Axodox::Threading::async_operation& operation)
  {
    auto huggingFaceClient = dependencies.resolve<HuggingFaceClient>();

    auto result = huggingFaceClient->TryDownloadModel(
      modelId,
      HuggingFaceModelDetails::StableDiffusionOnnxFileset,
      _root / modelId,
      operation);

    if (result)
    {
      ModelMetadata metadata;
      *metadata.Id = modelId;
      *metadata.Name = modelId;

      try_write_text(_root / modelId / "unpaint.json", stringify_json(metadata));
    }
    else
    {
      error_code ec;
      filesystem::remove_all(_root / modelId, ec);
    }

    Refresh();

    return result;
  }

  bool ModelRepository::AddModelFromDisk(const winrt::Windows::Storage::StorageFolder& folder)
  {
    auto token = StorageApplicationPermissions::FutureAccessList().Add(folder);
    auto modelId = "local/" + to_string(folder.Name());

    ModelMetadata metadata;
    *metadata.Id = modelId;
    *metadata.Name = to_string(folder.Name());
    *metadata.AccessToken = to_string(token);
    
    filesystem::create_directories(_root / modelId);
    try_write_text(_root / modelId / "unpaint.json", stringify_json(metadata));
    Refresh();

    return true;
  }

  void ModelRepository::UninstallModel(std::string_view modelId)
  {
    error_code ec;
    filesystem::remove_all(_root / modelId, ec);

    Refresh();
  }

  std::filesystem::path ModelRepository::Root() const
  {
    return _root;
  }

  std::set<ModelInfo> ModelRepository::Models() const
  {
    lock_guard lock(_mutex);
    return _models;
  }

  void ModelRepository::Refresh()
  {
    set<ModelInfo> models;

    error_code ec;
    for (auto& file : filesystem::recursive_directory_iterator{ _root, ec })
    {
      if (file.is_directory() || file.path().filename() != "unpaint.json") continue;

      auto text = try_read_text(file.path());
      if (!text) continue;

      auto metadata = try_parse_json<ModelMetadata>(*text);
      if (!metadata) continue;

      models.emplace(ModelInfo{ 
        *metadata->Id, 
        metadata->Name->empty() ? *metadata->Id : *metadata->Name,
        *metadata->AccessToken 
      });
    }

    lock_guard lock(_mutex);
    _models = models;
  }

  std::optional<ModelInfo> ModelRepository::GetModel(std::string_view modelId) const
  {
    lock_guard lock(_mutex);
    for (auto& model : _models)
    {
      if (model.Id == modelId) return model;
    }

    return nullopt;
  }

  std::unordered_map<std::string, winrt::Windows::Storage::StorageFile> ModelRepository::GetModelFiles(std::string_view modelId) const
  {
    auto model = GetModel(modelId);
    if (!model) return {};

    StorageFolder modelFolder{ nullptr };
    if (model->AccessToken.empty())
    {
      auto path = _root / model->Id;
      path.make_preferred();
      modelFolder = StorageFolder::GetFolderFromPathAsync(path.c_str()).get();
    }
    else
    {
      modelFolder = StorageApplicationPermissions::FutureAccessList().GetFolderAsync(to_hstring(model->AccessToken)).get();
    }

    vector<StorageFile> files;
    read_files_recursively(modelFolder, files).get();

    unordered_map<string, StorageFile> results;
    for (auto& file : files)
    {
      results.emplace(to_lower(to_string(file.Path()).substr(modelFolder.Path().size() + 1)), file);
    }

    return results;
  }

  ModelInfo::operator ModelViewModel() const
  {
    return ModelViewModel{
      .Id = to_hstring(Id),
      .Name = to_hstring(Name),
      .Uri = to_hstring("https://huggingface.co/" + Id)
    };
  }
}