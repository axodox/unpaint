#pragma once
#include "ShowcaseView.g.h"

namespace winrt::Unpaint::implementation
{
  struct ShowcaseView : ShowcaseViewT<ShowcaseView>
  {
    ShowcaseView();

    ShowcaseViewModel ViewModel();

  private:
    ShowcaseViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ShowcaseView : ShowcaseViewT<ShowcaseView, implementation::ShowcaseView>
  {
  };
}
