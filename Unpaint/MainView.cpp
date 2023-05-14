#include "pch.h"
#include "MainView.h"
#include "MainView.g.cpp"
#include "Infrastructure/WinRtDependencies.h"
#include "UnpaintSettings.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::ViewManagement; 
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  MainView::MainView() :
    _settingsManager(dependencies.resolve<SettingManager>()),
    _modelRepository(dependencies.resolve<ModelRepository>())
  {
    //Initialize main view
    InitializeComponent();
    InitializeTitleBar();

    dependencies.add<INavigationService>(*this);
    NavigateToView(xaml_typename<InferenceView>());
  }

  void MainView::NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType)
  {
    if (!_settingsManager->LoadSettingOr(Settings::UserInterface::HasShownWelcomeView, false))
    {
      viewType = xaml_typename<WelcomeView>();
    }
    else if (viewType == xaml_typename<InferenceView>() && _modelRepository->Models().empty())
    {
      viewType = xaml_typename<ModelsView>();
    }

    ContentFrame().Navigate(viewType);
  }

  void MainView::InitializeTitleBar()
  {
    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    coreTitleBar.ExtendViewIntoTitleBar(true);

    auto applicationView = ApplicationView::GetForCurrentView();
    applicationView.SetPreferredMinSize({ 1400, 840 });

    auto viewTitleBar = applicationView.TitleBar();
    viewTitleBar.ButtonBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonForegroundColor(Colors::White());
    viewTitleBar.ButtonInactiveBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonInactiveForegroundColor(Colors::White());

    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      UpdateTitleBar();
      });

    UpdateTitleBar();

    auto window = Window::Current();
    window.SetTitleBar(TitleBar());    
  }
  
  void MainView::UpdateTitleBar()
  {
    auto titleBar = CoreApplication::GetCurrentView().TitleBar();

    TitleBar().Height(titleBar.Height());
    TitleBarLeftPaddingColumn().Width({ titleBar.SystemOverlayLeftInset() });
    TitleBarRightPaddingColumn().Width({ titleBar.SystemOverlayRightInset() });
  }
}
