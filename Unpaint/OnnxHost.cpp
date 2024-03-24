#include "pch.h"
#include "OnnxHost.h"

using namespace Axodox::MachineLearning::Executors;
using namespace Axodox::MachineLearning::Sessions;
using namespace std;

namespace winrt::Unpaint
{
  OnnxHost::OnnxHost() :
    _environment(make_shared<OnnxEnvironment>()),
    _executor(make_shared<DmlExecutor>())
  { }

  const std::shared_ptr<Axodox::MachineLearning::Sessions::OnnxEnvironment>& OnnxHost::Environment() const
  {
    return _environment;
  }

  const std::shared_ptr<Axodox::MachineLearning::Executors::OnnxExecutor>& OnnxHost::Executor() const
  {
    return _executor;
  }

  Axodox::MachineLearning::Sessions::OnnxSessionParameters OnnxHost::ParametersFromFile(const std::filesystem::path& path) const
  {
    return { _environment, _executor, OnnxModelSource::FromFilePath(path) };
  }

  Axodox::MachineLearning::Sessions::OnnxSessionParameters OnnxHost::ParametersFromFile(const winrt::Windows::Storage::StorageFile& file) const
  {
    return { _environment, _executor, OnnxModelSource::FromStorageFile(file) };
  }
}