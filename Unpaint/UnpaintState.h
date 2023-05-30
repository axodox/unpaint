#pragma once
#include "StableDiffusionModelExecutor.h"

namespace winrt::Unpaint
{
  struct UnpaintState
  {
    InferenceMode InferenceMode = InferenceMode::Create;

    std::string ModelId;
    winrt::Windows::Graphics::SizeInt32 Resolution;
    std::string Project;
    std::string Image;

    bool IsJumpingToLatestImage = true;
    bool IsSettingsLocked = true;

    hstring PositivePrompt, NegativePrompt;

    bool IsBatchGenerationEnabled = false;
    uint32_t BatchSize = 8;

    float GuidanceStrength = 7.f, DenoisingStrength = 0.6f;
    uint32_t SamplingSteps = 15, RandomSeed = 0;
    bool IsSeedFrozen = false;
  };
}