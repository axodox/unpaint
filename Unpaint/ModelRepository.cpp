#include "pch.h"
#include "ModelRepository.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace Axodox::Web;
using namespace std;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  ModelMetadata::ModelMetadata() :
    Id(this, "id")
  { }

  ModelRepository::ModelRepository() :
    _root((ApplicationData::Current().LocalCacheFolder().Path() + L"/models").c_str())
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
    auto modelRepository = dependencies.resolve<ModelRepository>();
    auto huggingFaceClient = dependencies.resolve<HuggingFaceClient>();

    auto result = huggingFaceClient->TryDownloadModel(
      modelId,
      HuggingFaceModelDetails::StableDiffusionOnnxFileset,
      modelRepository->Root() / modelId,
      operation);

    if (result)
    {
      ModelMetadata metadata;
      *metadata.Id = modelId;

      try_write_text(modelRepository->Root() / modelId / "unpaint.json", stringify_json(metadata));
    }

    Refresh();

    return result;
  }

  std::filesystem::path ModelRepository::Root() const
  {
    return _root;
  }

  std::vector<std::string> ModelRepository::Models() const
  {
    lock_guard lock(_mutex);
    return _models;
  }

  void ModelRepository::Refresh()
  {
    vector<string> models;

    error_code ec;
    for (auto& file : filesystem::recursive_directory_iterator{ _root, ec })
    {
      if (file.is_directory() || file.path().filename() != "unpaint.json") continue;

      auto text = try_read_text(file.path());
      if (!text) continue;

      auto metadata = try_parse_json<ModelMetadata>(*text);
      if (!metadata) continue;

      models.push_back(*metadata->Id);
    }

    lock_guard lock(_mutex);
    _models = models;
  }
}