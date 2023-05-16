#pragma once
#include "SettingsView.g.h"

namespace winrt::Unpaint::implementation
{
  struct SettingsView : SettingsViewT<SettingsView>
  {
    SettingsView() = default;

    SettingsViewModel ViewModel();

  private:
    SettingsViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingsView : SettingsViewT<SettingsView, implementation::SettingsView>
  {
  };
}
