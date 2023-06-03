#pragma once
#include "SettingsView.g.h"

namespace winrt::Unpaint::implementation
{
  struct SettingsView : SettingsViewT<SettingsView>
  {
    SettingsView();

    SettingsViewModel ViewModel();

  private:
    SettingsViewModel _viewModel;

    Windows::ApplicationModel::Core::CoreApplicationViewTitleBar::LayoutMetricsChanged_revoker _titleBarLayoutMetricsChangedRevoker;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingsView : SettingsViewT<SettingsView, implementation::SettingsView>
  {
  };
}
