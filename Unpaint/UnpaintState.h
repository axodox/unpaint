#pragma once
#include "OptionProperty.h"
#include "DeviceInformation.h"

namespace winrt::Unpaint
{
  enum class InferenceMode
  {
    Create,
    Modify
  };

  class UnpaintState
  {
    Axodox::Infrastructure::event_owner _events;
    std::shared_ptr<DeviceInformation> _deviceInformation;

  public:

#pragma region Transient state
    OptionProperty<InferenceMode> InferenceMode{ InferenceMode::Create };

    OptionProperty<winrt::Windows::Graphics::SizeInt32> Resolution;
    OptionProperty<std::string> Project;
    OptionProperty<std::string> Image;

    OptionProperty<bool> IsJumpingToLatestImage{ true };
    OptionProperty<bool> IsSettingsLocked{ true };

    OptionProperty<std::string> PositivePrompt, NegativePrompt;

    OptionProperty<bool> IsBatchGenerationEnabled{ false };
    OptionProperty<uint32_t> BatchSize{ 8 };

    OptionProperty<float> GuidanceStrength{ 7.f }, DenoisingStrength{ 0.6f };
    OptionProperty<uint32_t> SamplingSteps{ 15 }, RandomSeed{ 0 };
    OptionProperty<bool> IsSeedFrozen{ false };

    OptionProperty<bool> IsControlNetEnabled;
    OptionProperty<std::string> ControlNetMode;
#pragma endregion

#pragma region Persistent state
    PersistentOptionProperty<bool> HasShownShowcaseView;
    PersistentOptionProperty<bool> HasShownWelcomeView;

    PersistentOptionProperty<bool> IsSafeModeEnabled;
    PersistentOptionProperty<bool> IsSafetyCheckerEnabled;

    PersistentOptionProperty<bool> IsDenoiserPinned;
    PersistentOptionProperty<uint32_t> AdapterIndex;
    PersistentOptionProperty<std::string> ModelId;
#pragma endregion

    Axodox::Infrastructure::event_publisher<OptionPropertyBase*> StateChanged;

    UnpaintState();

  private:
    void OnStateChanged(OptionPropertyBase* property);
  };
}