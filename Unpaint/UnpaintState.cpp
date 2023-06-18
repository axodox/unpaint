#include "pch.h"
#include "UnpaintState.h"

using namespace Axodox::Infrastructure;
using namespace winrt::Windows::Graphics;

namespace winrt::Unpaint
{
  UnpaintState::UnpaintState() :
    StateChanged(_events)
  {
    auto deviceInformation = dependencies.resolve<DeviceInformation>();

    //Set state change monitoring
    InferenceMode.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    Resolution.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    Project.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    Image.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    IsJumpingToLatestImage.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    IsSettingsLocked.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    PositivePrompt.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    NegativePrompt.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    IsBatchGenerationEnabled.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    BatchSize.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    GuidanceStrength.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    DenoisingStrength.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    SamplingSteps.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    RandomSeed.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });
    IsSeedFrozen.ValueChanged(no_revoke, event_handler{ this, &UnpaintState::OnStateChanged });

    //Initialize properties
    if (*Resolution == SizeInt32{0, 0})
    {
      Resolution = deviceInformation->IsDeviceXbox() ? SizeInt32{ 512, 512 } : SizeInt32{ 768, 768 };
    }
  }

  void UnpaintState::OnStateChanged(OptionPropertyBase* property)
  {
    _events.raise(StateChanged, property);
  }
}