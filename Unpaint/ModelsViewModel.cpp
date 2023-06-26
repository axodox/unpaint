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
using namespace winrt::Windows::ApplicationModel::DataTransfer;
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
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsModelWebsiteAvailable"));
  }

  bool ModelsViewModel::IsInstalledModelSelected()
  {
    return _selectedInstalledModel >= 0 && _selectedInstalledModel < int32_t(_installedModels.Size());
  }

  bool ModelsViewModel::IsModelWebsiteAvailable()
  {
    return IsInstalledModelSelected() && !_installedModels.GetAt(_selectedInstalledModel).Uri.empty();
  }

  fire_and_forget ModelsViewModel::OpenModelDirectory()
  {
    auto modelId = to_string(_installedModels.GetAt(_selectedInstalledModel).Id);

    auto lifetime = get_strong();
    auto modelFolder = co_await _modelRepository->GetModelFolderAsync(modelId);
    if (!modelFolder) co_return;

    co_await Launcher::LaunchFolderAsync(modelFolder);
  }

  void ModelsViewModel::OpenInstalledModelWebsite()
  {
    auto uri = _installedModels.GetAt(_selectedInstalledModel).Uri;
    Launcher::LaunchUriAsync(Uri(uri));
  }

  void ModelsViewModel::CopyLinkToClipboard()
  {
    auto modelId = _installedModels.GetAt(_selectedInstalledModel).Id;

    Uri result{ format(L"unpaint://models/install?id={}", Uri::EscapeComponent(modelId)) };

    DataPackage dataPackage{};
    dataPackage.SetText(result.ToString());
    Clipboard::SetContent(dataPackage);
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

  bool ModelsViewModel::CanContinue()
  {
    return !AreInstalledModelsEmpty();
  }

  void ModelsViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<InferenceView>());
  }

  fire_and_forget ModelsViewModel::OpenUri(Windows::Foundation::Uri const& uri)
  {
    if (uri.Path() != L"/install") co_return;

    hstring modelId;
    auto query = uri.QueryParsed();
    for (const auto& item : query)
    {
      if (item.Name() == L"id")
      {
        modelId = item.Value();
      }
    }

    if (modelId.empty()) co_return; 
    
    auto lifetime = get_strong();

    ContentDialog confirmationDialog{};
    confirmationDialog.Title(box_value(L"Installing model"));
    confirmationDialog.Content(box_value(format(L"Would you like to import model {}?", modelId)));
    confirmationDialog.PrimaryButtonText(L"Yes");
    confirmationDialog.SecondaryButtonText(L"No");
    confirmationDialog.DefaultButton(ContentDialogButton::Primary);

    auto confirmationResult = co_await confirmationDialog.ShowAsync();
    if (ContentDialogResult::Primary != confirmationResult) co_return;
    
    DownloadHuggingFaceModelAsync(modelId);
  }

  fire_and_forget ModelsViewModel::OpenControlNetSettingsAsync()
  {
    auto lifetime = get_strong();

    ControlNetModelsDialog controlnetDialog{};

    auto result = co_await controlnetDialog.ShowAsync();
    if (result != ContentDialogResult::Primary) co_return;

    DownloadModelDialog downloadDialog{};
    downloadDialog.ViewModel().EnsureControlNetModelsAsync(controlnetDialog.ViewModel().SelectedModes());

    co_await downloadDialog.ShowAsync();
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
    auto lifetime = get_strong();

    if (!_modelRepository->GetModel(to_string(modelId)))
    {
      DownloadModelDialog dialog{};
      dialog.ViewModel().DownloadStableDiffusionModelAsync(modelId);

      co_await dialog.ShowAsync();

      UpdateInstalledModels();
    }
    else
    {
      ContentDialog messageDialog{};
      messageDialog.Title(box_value(L"Importing model"));
      messageDialog.Content(box_value(L"The selected model is already installed."));
      messageDialog.PrimaryButtonText(L"OK");
      messageDialog.DefaultButton(ContentDialogButton::Primary);

      co_await messageDialog.ShowAsync();
    }
  }
}
