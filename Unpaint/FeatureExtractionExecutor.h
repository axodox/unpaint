#pragma once
#include "pch.h"
#include "UnpaintState.h"
#include "OnnxHost.h"

namespace winrt::Unpaint
{
  enum class FeatureExtractionMode
  {
    Unknown,
    Canny,
    Hed,
    Depth,
    OpenPose
  };

  class FeatureExtractionExecutor
  {
  public:
    FeatureExtractionExecutor();

    Axodox::Graphics::TextureData ExtractFeatures(const Axodox::Graphics::TextureData& sourceImage, FeatureExtractionMode mode, Axodox::Threading::async_operation& operation);

    static FeatureExtractionMode ParseExtractionMode(std::string_view text);

  private:
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<OnnxHost> _onnxHost;

    FeatureExtractionMode _mode;
    std::unique_ptr<Axodox::MachineLearning::Imaging::Annotators::ImageAnnotator> _featureExtractor;

    Axodox::Graphics::TextureData _input, _output;

    void EnsureExtractor(FeatureExtractionMode mode, Axodox::Threading::async_operation_source& async);
  };
}