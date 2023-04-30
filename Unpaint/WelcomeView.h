#pragma once
#include "WelcomeView.g.h"

namespace winrt::Unpaint::implementation
{
  struct WelcomeView : WelcomeViewT<WelcomeView>
  {
    WelcomeView() = default;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct WelcomeView : WelcomeViewT<WelcomeView, implementation::WelcomeView>
  {
  };
}
