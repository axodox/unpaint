#include "pch.h"
#include "SettingsView.h"
#include "SettingsView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  SettingsView::SettingsView()
  {
    InitializeComponent();

    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      Margin(Thickness{ 0.f, coreTitleBar.Height(), 0.f, 0.f });
    });
    Margin(Thickness{ 0.f, coreTitleBar.Height(), 0.f, 0.f });
  }

  SettingsViewModel SettingsView::ViewModel()
  {
    return _viewModel;
  }
}
