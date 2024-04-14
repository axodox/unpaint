#include "pch.h"
#include "FeatureExtractionExecutor.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning::Imaging::Annotators;
using namespace Axodox::MachineLearning::Sessions;
using namespace Axodox::Threading;
using namespace std;
using namespace std::filesystem;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  FeatureExtractionExecutor::FeatureExtractionExecutor() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _onnxHost(dependencies.resolve<OnnxHost>())
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

      if (!_unpaintState->IsFeatureExtractorPinned) _featureExtractor.reset();
      
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
    //Check if models are usable
    if (_featureExtractor && _mode == mode) return;

    //Reset models
    async.update_state("Loading feature extractor...");
    _featureExtractor.reset();

    //Load model
    auto root = path{ ApplicationData::Current().LocalCacheFolder().Path().c_str() } / "annotators";

    switch (mode)
    {
    case FeatureExtractionMode::Canny:
      _featureExtractor = make_unique<EdgeDetector>(_onnxHost->ParametersFromFile(root / "canny.onnx"));
      break;
    case FeatureExtractionMode::Hed:
      _featureExtractor = make_unique<EdgeDetector>(_onnxHost->ParametersFromFile(root / "hed.onnx"));
      break;
    case FeatureExtractionMode::Depth:
      _featureExtractor = make_unique<DepthEstimator>(_onnxHost->ParametersFromFile(root / "depth.onnx"));
      break;
    case FeatureExtractionMode::OpenPose:
      _featureExtractor = make_unique<PoseEstimator>(_onnxHost->ParametersFromFile(root / "openpose.onnx"));
      break;
    default:
      throw logic_error("Feature extraction mode not implemented!");
    }

    _mode = mode;
  }
}
