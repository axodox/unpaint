#pragma once
#include "ModelsViewModel.g.h"
#include "ModelRepository.h"

namespace winrt::Unpaint::implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel>
  {
    ModelsViewModel();

    fire_and_forget ImportModelFromHuggingFaceAsync();

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> InstalledModels();
    bool AreInstalledModelsEmpty();
    fire_and_forget RemoveModelAsync();
    void OpenInstalledModelWebsite();

    int32_t SelectedInstalledModel();
    void SelectedInstalledModel(int32_t value);
    bool IsInstalledModelSelected();
    fire_and_forget OpenModelDirectory();

    bool CanContinue();
    void Continue();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    static const char* const _modelFilter;
    INavigationService _navigationService;
    std::shared_ptr<ModelRepository> _modelRepository;
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> _installedModels;
    int32_t _selectedInstalledModel = -1;

    void UpdateInstalledModels();

    static ModelViewModel CreateModelViewModel(const std::string& modelId);
    fire_and_forget DownloadHuggingFaceModelAsync(hstring const& modelId);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ModelsViewModel : ModelsViewModelT<ModelsViewModel, implementation::ModelsViewModel>
  {
  };
}
