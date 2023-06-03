#include "pch.h"
#include "ModelsViewModel.h"
#include "ModelsViewModel.g.cpp"
#include "Infrastructure/WinRtDependencies.h"
#include "Infrastructure/Win32.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Web;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  const char* const ModelsViewModel::_modelFilter = "unpaint,stable_diffusion_model";

  ModelsViewModel::ModelsViewModel() :
    _availableModels(single_threaded_observable_vector<ModelViewModel>()),
    _installedModels(single_threaded_observable_vector<ModelViewModel>()),
    _navigationService(dependencies.resolve<INavigationService>()),
    _modelRepository(dependencies.resolve<ModelRepository>())
  {
    UpdateInstalledModels();
    UpdateAvailableModelsAsync();
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::AvailableModels()
  {
    return _availableModels;
  }

  bool ModelsViewModel::AreAvailableModelsEmpty()
  {
    return _availableModels.Size() == 0;
  }

  fire_and_forget ModelsViewModel::UpdateAvailableModelsAsync()
  {
    apartment_context callerContext;
    
    auto lifetime = get_strong();
    co_await resume_background();

    HuggingFaceClient client{};
    auto models = client.GetModels(_modelFilter);
    auto installedModels = _modelRepository->Models();

    co_await callerContext;

    _availableModels.Clear();
    for (auto& model : models)
    {
      if (installedModels.contains(model)) continue;

#ifdef NDEBUG
      //Sorry mates I do not trust you this much...
      if (!model.starts_with("axodoxian/")) continue;
#endif

      _availableModels.Append(CreateModelViewModel(model));
    }

    _propertyChanged(*this, PropertyChangedEventArgs(L"AreAvailableModelsEmpty"));
  }

  fire_and_forget ModelsViewModel::DownloadModelAsync()
  {
    auto modelId = _availableModels.GetAt(_selectedAvailableModel).Id;
    DownloadModelDialog dialog{ modelId };

    auto lifetime = get_strong();
    co_await dialog.ShowAsync();

    UpdateInstalledModels();
    UpdateAvailableModelsAsync();
  }

  void ModelsViewModel::OpenAvailableModelWebsite()
  {
    auto uri = _availableModels.GetAt(_selectedAvailableModel).Uri;
    Launcher::LaunchUriAsync(Uri(uri));
  }

  int32_t ModelsViewModel::SelectedAvailableModel()
  {
    return _selectedAvailableModel;
  }

  void ModelsViewModel::SelectedAvailableModel(int32_t value)
  {
    if (value == _selectedAvailableModel) return;

    _selectedAvailableModel = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedAvailableModel"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsAvailableModelSelected"));
  }

  bool ModelsViewModel::IsAvailableModelSelected()
  {
    return _selectedAvailableModel != -1;
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::InstalledModels()
  {
    return _installedModels;
  }

  bool ModelsViewModel::AreInstalledModelsEmpty()
  {
    return _installedModels.Size() == 0;
  }

  fire_and_forget ModelsViewModel::RemoveModelAsync()
  {
    auto modelId = to_string(_installedModels.GetAt(_selectedInstalledModel).Id);

    ContentDialog confirmationDialog{};
    confirmationDialog.Title(box_value(L"Uninstalling model"));
    confirmationDialog.Content(box_value(to_wstring(format("Would you like to remove model {}?", modelId))));
    confirmationDialog.PrimaryButtonText(L"Yes");
    confirmationDialog.SecondaryButtonText(L"No");

    auto lifetime = get_strong();
    auto result = co_await confirmationDialog.ShowAsync();

    if (result == ContentDialogResult::Primary)
    {
      _modelRepository->UninstallModel(modelId);

      UpdateInstalledModels();
      UpdateAvailableModelsAsync();
    }
  }

  void ModelsViewModel::OpenInstalledModelWebsite()
  {
    auto uri = _installedModels.GetAt(_selectedInstalledModel).Uri;
    Launcher::LaunchUriAsync(Uri(uri));
  }

  int32_t ModelsViewModel::SelectedInstalledModel()
  {
    return _selectedInstalledModel;
  }

  void ModelsViewModel::SelectedInstalledModel(int32_t value)
  {
    if (value == _selectedInstalledModel) return;

    _selectedInstalledModel = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedInstalledModel"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsInstalledModelSelected"));
  }

  bool ModelsViewModel::IsInstalledModelSelected()
  {
    return _selectedInstalledModel != -1;
  }

  fire_and_forget ModelsViewModel::OpenModelDirectory()
  {
    auto lifetime = get_strong();
    auto path = _modelRepository->Root();
    auto modelFolder = co_await StorageFolder::GetFolderFromPathAsync(path.c_str());
    co_await Launcher::LaunchFolderAsync(modelFolder);
  }

  bool ModelsViewModel::CanContinue()
  {
    return !AreInstalledModelsEmpty();
  }

  void ModelsViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<InferenceView>());
  }

  event_token ModelsViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void ModelsViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
  
  void ModelsViewModel::UpdateInstalledModels()
  {
    _modelRepository->Refresh();

    _installedModels.Clear();
    for (auto& model : _modelRepository->Models())
    {
      _installedModels.Append(CreateModelViewModel(model));
    }

    _propertyChanged(*this, PropertyChangedEventArgs(L"AreInstalledModelsEmpty"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"CanContinue"));
  }
  
  ModelViewModel ModelsViewModel::CreateModelViewModel(const std::string& modelId)
  {
    return ModelViewModel{
      .Id = to_hstring(modelId),
      .Uri = to_hstring("https://huggingface.co/" + modelId)
    };
  }
}
