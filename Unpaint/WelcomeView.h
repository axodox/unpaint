#pragma once
#include "WelcomeView.g.h"

namespace winrt::Unpaint::implementation
{
  struct WelcomeView : WelcomeViewT<WelcomeView>
  {
    WelcomeView() = default;

    void OnContinueClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct WelcomeView : WelcomeViewT<WelcomeView, implementation::WelcomeView>
  {
  };
}
