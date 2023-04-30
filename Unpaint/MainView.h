#pragma once
#include "MainView.g.h"

namespace winrt::Unpaint::implementation
{
  struct MainView : MainViewT<MainView>
  {
    MainView();

  private:
    Windows::ApplicationModel::Core::CoreApplicationViewTitleBar::LayoutMetricsChanged_revoker _titleBarLayoutMetricsChangedRevoker;

    void InitializeTitleBar();
    void UpdateTitleBar();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct MainView : MainViewT<MainView, implementation::MainView>
  {
  };
}
