#pragma once
#include "StableDiffusionModelExecutor.h"
#include "OptionProperty.h"

namespace winrt::Unpaint
{
  class UnpaintState
  {
    Axodox::Infrastructure::event_owner _events;

  public:
    OptionProperty<InferenceMode> InferenceMode = InferenceMode::Create;

    OptionProperty<winrt::Windows::Graphics::SizeInt32> Resolution;
    OptionProperty<std::string> Project;
    OptionProperty<std::string> Image;

    OptionProperty<bool> IsJumpingToLatestImage = true;
    OptionProperty<bool> IsSettingsLocked = true;

    OptionProperty<hstring> PositivePrompt, NegativePrompt;

    OptionProperty<bool> IsBatchGenerationEnabled = false;
    OptionProperty<uint32_t> BatchSize = 8;

    OptionProperty<float> GuidanceStrength = 7.f, DenoisingStrength = 0.6f;
    OptionProperty<uint32_t> SamplingSteps = 15, RandomSeed = 0;
    OptionProperty<bool> IsSeedFrozen = false;
    
    Axodox::Infrastructure::event_publisher<OptionPropertyBase*> StateChanged;

    UnpaintState();

  private:
    void OnStateChanged(OptionPropertyBase* property);
  };
}