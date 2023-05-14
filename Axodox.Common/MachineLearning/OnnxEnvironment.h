#pragma once
#ifdef USE_ONNX
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
    Ort::SessionOptions DefaultSessionOptions();
    Ort::SessionOptions CpuSessionOptions();

  private:
    std::filesystem::path _rootPath;
    Ort::Env _environment;
    Ort::MemoryInfo _memoryInfo;
  };
}
#endif