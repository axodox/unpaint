#include "pch.h"
#include "SettingsView.h"
#include "SettingsView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  SettingsViewModel SettingsView::ViewModel()
  {
    return _viewModel;
  }
}
