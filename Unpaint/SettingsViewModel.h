﻿#pragma once
#include "SettingsViewModel.g.h"
#include "UnpaintState.h"

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

    Windows::Foundation::Collections::IObservableVector<AdapterViewModel> Adapters();
    int32_t SelectedAdapterIndex();
    void SelectedAdapterIndex(int32_t value);

    void Continue();

  private:
    INavigationService _navigationService;
    std::shared_ptr<UnpaintState> _unpaintState;

    Windows::Foundation::Collections::IObservableVector<AdapterViewModel> _adapters;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingsViewModel : SettingsViewModelT<SettingsViewModel, implementation::SettingsViewModel>
  {
  };
}
