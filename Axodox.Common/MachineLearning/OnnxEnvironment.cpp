#include "pch.h"
#ifdef USE_ONNX
#include "OnnxEnvironment.h"

using namespace Ort;

namespace Axodox::MachineLearning
{
  OnnxEnvironment::OnnxEnvironment(const std::filesystem::path& rootPath) :
    _rootPath(rootPath),
    _environment(),
    _memoryInfo(MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault))
  {
    _environment.UpdateEnvWithCustomLogLevel(ORT_LOGGING_LEVEL_WARNING);
    
    _defaultSessionOptions.DisableMemPattern();
    _defaultSessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
    _defaultSessionOptions.SetGraphOptimizationLevel(ORT_DISABLE_ALL);

    //OrtSessionOptionsAppendExecutionProvider_CUDA(_defaultSessionOptions, 0);
    OrtSessionOptionsAppendExecutionProvider_DML(_defaultSessionOptions, 0);

    _cpuSessionOptions.DisableMemPattern();
    _cpuSessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
    _cpuSessionOptions.SetGraphOptimizationLevel(ORT_DISABLE_ALL);
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

  Ort::SessionOptions& OnnxEnvironment::DefaultSessionOptions()
  {
    return _defaultSessionOptions;
  }
  
  Ort::SessionOptions& OnnxEnvironment::CpuSessionOptions()
  {
    return _cpuSessionOptions;
  }
}
#endif