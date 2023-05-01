#pragma once
#include "pch.h"
#include "Json/JsonSerializer.h"
#include "Threading/AsyncOperation.h"

namespace winrt::Unpaint
{
  struct ModelMetadata : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<std::string> Id;

    ModelMetadata();
  };

  class ModelRepository
  {
  public:
    ModelRepository();

    bool TryInstallHuggingFaceModel(std::string_view modelId, Axodox::Threading::async_operation& operation);

    std::filesystem::path Root() const;
    std::vector<std::string> Models() const;
    void Refresh();

  private:
    mutable std::mutex _mutex;
    std::filesystem::path _root;
    std::vector<std::string> _models;
  };
}