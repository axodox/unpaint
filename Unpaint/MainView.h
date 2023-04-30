#pragma once
#include "MainView.g.h"

namespace winrt::Unpaint::implementation
{
  struct MainView : MainViewT<MainView>
  {
    MainView() = default;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct MainView : MainViewT<MainView, implementation::MainView>
  {
  };
}
