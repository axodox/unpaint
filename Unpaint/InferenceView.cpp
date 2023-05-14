#include "pch.h"
#include "InferenceView.h"
#include "InferenceView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  InferenceView::InferenceView()
  {
    InitializeComponent();

    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      StatusBar().Height(coreTitleBar.Height());
    });
    StatusBar().Height(coreTitleBar.Height());
  }

  InferenceViewModel InferenceView::ViewModel()
  {
    return _viewModel;
  }
}
