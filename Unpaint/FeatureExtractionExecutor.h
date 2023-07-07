#pragma once
#include "pch.h"

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
    Axodox::Graphics::TextureData ExtractFeatures(const Axodox::Graphics::TextureData& sourceImage, FeatureExtractionMode mode, Axodox::Threading::async_operation& operation);

    static FeatureExtractionMode ParseExtractionMode(std::string_view text);

  private:
    FeatureExtractionMode _mode;
    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _environment;
    std::unique_ptr<Axodox::MachineLearning::ImageFeatureExtractor> _featureExtractor;

    void EnsureExtractor(FeatureExtractionMode mode, Axodox::Threading::async_operation_source& async);
  };
}