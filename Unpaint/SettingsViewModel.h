#pragma once
#include "SettingsViewModel.g.h"
#include "UnpaintOptions.h"

namespace winrt::Unpaint::implementation
{
  struct SettingsViewModel : SettingsViewModelT<SettingsViewModel>
  {
    SettingsViewModel();

    bool AreUnsafeOptionsEnabled();

    bool IsSafeModeEnabled();
    void IsSafeModeEnabled(bool value);

    bool IsSafetyCheckerEnabled();
    void IsSafetyCheckerEnabled(bool value);

    bool IsDenoiserPinned();
    void IsDenoiserPinned(bool value);

    hstring Version();
    Windows::Foundation::Uri SelectedModelUri();

    void Continue();

  private:
    INavigationService _navigationService;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingsViewModel : SettingsViewModelT<SettingsViewModel, implementation::SettingsViewModel>
  {
  };
}
