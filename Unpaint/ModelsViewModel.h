#pragma once
#include "ModelsViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel>
  {
    ModelsViewModel();

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> AvailableModels();
    fire_and_forget UpdateAvailableModelsAsync();

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> InstalledModels();

  private:
    static const char* const _modelFilter;

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> _availableModels;
    Windows::Foundation::Collections::IObservableVector<ModelViewModel> _installedModels;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel, implementation::ModelsViewModel>
  {
  };
}
