#pragma once
#include "ModelsView.g.h"

namespace winrt::Unpaint::implementation
{
  struct ModelsView : ModelsViewT<ModelsView>
  {
    ModelsView() = default;

    ModelsViewModel ViewModel();

  private:
    ModelsViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ModelsView : ModelsViewT<ModelsView, implementation::ModelsView>
  {
  };
}
