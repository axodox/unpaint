#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "Threading/AsyncOperation.h"

namespace winrt::Unpaint
{
  struct StableDiffusionInferenceTask
  {
    std::wstring PositivePrompt, NegativePrompt;
    DirectX::XMUINT2 Resolution;

    float GuidanceStrength;
    uint32_t SamplingSteps;
    uint32_t RandomSeed;

    std::string ModelId;
  };

  class StableDiffusionModelExecutor
  {
  public:
    StableDiffusionModelExecutor();

    Axodox::Graphics::TextureData TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation);

  private:

  };
}