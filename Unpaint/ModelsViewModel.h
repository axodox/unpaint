#pragma once
#include "ModelsViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel>
  {
    ModelsViewModel();

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> AvailableModels();
    fire_and_forget UpdateAvailableModelsAsync();

  private:
    static const char* const _modelFilter;

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> _availableModels;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel, implementation::ModelsViewModel>
  {
  };
}
