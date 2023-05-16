#pragma once
#include "MainView.g.h"
#include "UnpaintOptions.h"
#include "ModelRepository.h"

namespace winrt::Unpaint::implementation
{
  struct MainView : MainViewT<MainView>
  {
    MainView();

    void NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType);

    bool IsPointerOverTitleBar();

    event_token IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler);
    void IsPointerOverTitleBarChanged(event_token const& token) noexcept;

  private:
    event<Windows::Foundation::EventHandler<bool>> _isPointerOverTitleBarChanged;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;
    bool _isPointerOverTitleBar;

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
