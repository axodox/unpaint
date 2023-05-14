#include "pch.h"
#ifdef USE_ONNX
#include "OnnxEnvironment.h"

using namespace Ort;

namespace Axodox::MachineLearning
{
  OnnxEnvironment::OnnxEnvironment(const std::filesystem::path& rootPath) :
    _rootPath(rootPath),
    _environment(),
    _memoryInfo(MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
  {
    _environment.UpdateEnvWithCustomLogLevel(ORT_LOGGING_LEVEL_WARNING);    
  }

  const std::filesystem::path& OnnxEnvironment::RootPath() const
  {
    return _rootPath;
  }

  Ort::Env& OnnxEnvironment::Environment()
  {
    return _environment;
  }

  Ort::MemoryInfo& OnnxEnvironment::MemoryInfo()
  {
    return _memoryInfo;
  }

  Ort::SessionOptions OnnxEnvironment::DefaultSessionOptions()
  {
    Ort::SessionOptions options;
    options.DisableMemPattern();
    options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
    OrtSessionOptionsAppendExecutionProvider_DML(options, 0);
    return options;
  }
  
  Ort::SessionOptions OnnxEnvironment::CpuSessionOptions()
  {
    Ort::SessionOptions options;
    options.DisableMemPattern();
    options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
    return options;
  }
}
#endif