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
    _installedModels(single_threaded_observable_vector<ModelViewModel>()),
    _navigationService(dependencies.resolve<INavigationService>()),
    _modelRepository(dependencies.resolve<ModelRepository>())
  {
    UpdateInstalledModels();
  }
    
  fire_and_forget ModelsViewModel::ImportModelFromHuggingFaceAsync()
  {
    ImportHuggingFaceModelDialog dialog{};
    auto result = co_await dialog.ShowAsync();
    if (result != ContentDialogResult::Primary) co_return;

    auto viewModel = dialog.ViewModel();
    if (viewModel.IsValid()) DownloadHuggingFaceModelAsync(viewModel.ModelId());
  }

  fire_and_forget ModelsViewModel::ImportModelFromDiskAsync()
  {
    ImportLocalModelDialog dialog{};
    auto result = co_await dialog.ShowAsync();
    if (result != ContentDialogResult::Primary) co_return;

    auto modelFolder = dialog.ViewModel().Result();
    _modelRepository->AddModelFromDisk(modelFolder);

    UpdateInstalledModels();
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
      _installedModels.Append(model);
    }

    _propertyChanged(*this, PropertyChangedEventArgs(L"AreInstalledModelsEmpty"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"CanContinue"));
  }

  fire_and_forget ModelsViewModel::DownloadHuggingFaceModelAsync(hstring const& modelId)
  {
    DownloadModelDialog dialog{ modelId };

    auto lifetime = get_strong();
    co_await dialog.ShowAsync();

    UpdateInstalledModels();
  }
}
