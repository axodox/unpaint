#include "pch.h"
#include "MainView.h"
#include "MainView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::ViewManagement; 
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  MainView::MainView()
  {
    InitializeComponent();
    InitializeTitleBar();
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
