#pragma once
#include "pch.h"

namespace Axodox::MachineLearning
{
  class OnnxEnvironment
  {
  public:
    OnnxEnvironment(const std::filesystem::path& rootPath);

    const std::filesystem::path& RootPath() const;
    Ort::Env& Environment();
    Ort::MemoryInfo& MemoryInfo();
    Ort::SessionOptions& DefaultSessionOptions();

  private:
    std::filesystem::path _rootPath;
    Ort::Env _environment;
    Ort::MemoryInfo _memoryInfo;
    Ort::SessionOptions _defaultSessionOptions;
  };
}