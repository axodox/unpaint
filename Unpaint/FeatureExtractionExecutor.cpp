#include "pch.h"
#include "FeatureExtractionExecutor.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning;
using namespace Axodox::Threading;
using namespace std;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  FeatureExtractionExecutor::FeatureExtractionExecutor() :
    _unpaintState(dependencies.resolve<UnpaintState>())
  { }

  Axodox::Graphics::TextureData FeatureExtractionExecutor::ExtractFeatures(const Axodox::Graphics::TextureData& input, FeatureExtractionMode mode, Axodox::Threading::async_operation& operation)
  {
    async_operation_source async;
    operation.set_source(async);
   
    if (input != _input || mode != _mode || !_output)
    {
      try
      {
        _input = TextureData{ input };

        EnsureExtractor(mode, async);

        async.update_state("Extracting features...");
        _output = _featureExtractor->ExtractFeatures(input);
      }
      catch (...)
      {
        _output = {};
      }

      if (!_unpaintState->IsFeatureExtractorPinned) ReleaseExtractor();
    }

    return TextureData{ _output };
  }

  FeatureExtractionMode FeatureExtractionExecutor::ParseExtractionMode(std::string_view text)
  {
    if (text == "canny") return FeatureExtractionMode::Canny;
    if (text == "hed") return FeatureExtractionMode::Hed;
    if (text == "depth") return FeatureExtractionMode::Depth;
    if (text == "openpose") return FeatureExtractionMode::OpenPose;

    return FeatureExtractionMode::Unknown;
  }

  void FeatureExtractionExecutor::EnsureExtractor(FeatureExtractionMode mode, async_operation_source& async)
  {
    if (_featureExtractor && _mode == mode) return;

    async.update_state("Loading feature extractor...");
    if (!_environment) 
    {
      auto host = dependencies.resolve<OnnxHost>();
      _environment = make_unique<OnnxEnvironment>(host, ApplicationData::Current().LocalCacheFolder().Path().c_str());
    }

    _featureExtractor.reset();
    _mode = mode;

    switch (mode)
    {
    case FeatureExtractionMode::Canny:
      _featureExtractor = make_unique<EdgeDetector>(*_environment, EdgeDetectionMode::Canny);
      break;
    case FeatureExtractionMode::Hed:
      _featureExtractor = make_unique<EdgeDetector>(*_environment, EdgeDetectionMode::Hed);
      break;
    case FeatureExtractionMode::Depth:
      _featureExtractor = make_unique<DepthEstimator>(*_environment);
      break;
    case FeatureExtractionMode::OpenPose:
      _featureExtractor = make_unique<PoseEstimator>(*_environment);
      break;
    default:
      throw logic_error("Feature extraction mode not implemented!");
    }
  }

  void FeatureExtractionExecutor::ReleaseExtractor()
  {
    _featureExtractor.reset();
    _environment.reset();
  }
}
