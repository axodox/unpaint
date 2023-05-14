#pragma once
#include "MainView.g.h"
#include "Storage/SettingManager.h"
#include "ModelRepository.h"

namespace winrt::Unpaint::implementation
{
  struct MainView : MainViewT<MainView>
  {
    MainView();

    void NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType);

  private:
    std::shared_ptr<Axodox::Storage::SettingManager> _settingsManager;
    std::shared_ptr<ModelRepository> _modelRepository;

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
