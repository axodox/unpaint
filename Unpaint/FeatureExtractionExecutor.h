#pragma once
#include "pch.h"
#include "UnpaintState.h"

namespace winrt::Unpaint
{
  enum class FeatureExtractionMode
  {
    Unknown,
    Canny,
    Hed,
    Depth
  };

  class FeatureExtractionExecutor
  {
  public:
    FeatureExtractionExecutor();

    Axodox::Graphics::TextureData ExtractFeatures(const Axodox::Graphics::TextureData& sourceImage, FeatureExtractionMode mode, Axodox::Threading::async_operation& operation);

    static FeatureExtractionMode ParseExtractionMode(std::string_view text);

  private:
    std::shared_ptr<UnpaintState> _unpaintState;

    FeatureExtractionMode _mode;
    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _environment;
    std::unique_ptr<Axodox::MachineLearning::ImageFeatureExtractor> _featureExtractor;

    Axodox::Graphics::TextureData _input, _output;

    void EnsureExtractor(FeatureExtractionMode mode, Axodox::Threading::async_operation_source& async);
    void ReleaseExtractor();
  };
}