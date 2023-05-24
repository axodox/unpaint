#include "pch.h"
#include "InferenceViewModel.h"
#include "InferenceViewModel.g.cpp"
#include "Collections/ObservableExtensions.h"
#include "Graphics/Textures/TextureData.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/WinRtDependencies.h"
#include "Storage/UwpStorage.h"
#include "Storage/FileIO.h"
#include "Threading/AsyncOperation.h"
#include "StringMapper.h"

using namespace Axodox::Collections;
using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;

namespace winrt::Unpaint::implementation
{
  InferenceViewModel::InferenceViewModel() :
    _unpaintOptions(dependencies.resolve<UnpaintOptions>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _modelExecutor(dependencies.resolve<StableDiffusionModelExecutor>()),
    _imageRepository(dependencies.resolve<ImageRepository>()),
    _navigationService(dependencies.resolve<INavigationService>()),
    _inferenceMode(InferenceMode::Create),
    _isSettingsLocked(true),
    _guidanceStrength(7.f),
    _denoisingStrength(0.2f),
    _models(single_threaded_observable_vector<hstring>()),
    _resolutions(single_threaded_observable_vector<SizeInt32>()),
    _selectedResolutionIndex(1),
    _samplingSteps(15),
    _random(uint32_t(time(nullptr))),
    _isSeedFrozen(false),
    _status(L""),
    _progress(0),
    _images(single_threaded_observable_vector<hstring>()),
    _projects(single_threaded_observable_vector<hstring>()),
    _outputImage(nullptr),
    _inputImage(nullptr),
    _imagesChangedSubscription(_imageRepository->ImagesChanged(event_handler{ this, &InferenceViewModel::OnImagesChanged }))
  {
    for (auto& model : _modelRepository->Models())
    {
      _models.Append(to_hstring(model));
    }

    _resolutions.Append(SizeInt32{ 1024, 1024 });
    _resolutions.Append(SizeInt32{ 768, 768 });
    _resolutions.Append(SizeInt32{ 512, 512 });

    _imageRepository->Refresh();
    SelectedImageIndex(int32_t(_images.Size()) - 1);
  }

  int32_t InferenceViewModel::SelectedModeIndex()
  {
    return static_cast<int32_t>(_inferenceMode);
  }

  void InferenceViewModel::SelectedModeIndex(int32_t value)
  {
    if (static_cast<InferenceMode>(value) == _inferenceMode) return;

    _inferenceMode = static_cast<InferenceMode>(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedModeIndex"));
  }

  bool InferenceViewModel::IsSettingsLocked()
  {
    return _isSettingsLocked;
  }

  void InferenceViewModel::IsSettingsLocked(bool value)
  {
    if (value == _isSettingsLocked) return;

    _isSettingsLocked = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsSettingsLocked"));

    LoadImageMetadataAsync();
  }

  hstring InferenceViewModel::PositivePromptPlaceholder()
  {
    return L"an empty canvas standing in a painter's workshop";
  }

  hstring InferenceViewModel::PositivePrompt()
  {
    return _positivePrompt;
  }

  void InferenceViewModel::PositivePrompt(hstring const& value)
  {
    if (value == _positivePrompt) return;

    _positivePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"PositivePrompt"));
  }

  hstring InferenceViewModel::NegativePromptPlaceholder()
  {
    return L"blurry, render";
  }

  hstring InferenceViewModel::NegativePrompt()
  {
    return _negativePrompt;
  }

  void InferenceViewModel::NegativePrompt(hstring const& value)
  {
    if (value == _negativePrompt) return;

    _negativePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"NegativePrompt"));
  }

  Windows::Foundation::Collections::IObservableVector<hstring> InferenceViewModel::Models()
  {
    return _models;
  }

  int32_t InferenceViewModel::SelectedModelIndex()
  {
    return _selectedModelIndex;
  }

  void InferenceViewModel::SelectedModelIndex(int32_t value)
  {
    if (value == _selectedModelIndex) return;

    _selectedModelIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedModelIndex"));
  }

  Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> InferenceViewModel::Resolutions()
  {
    return _resolutions;
  }

  int32_t InferenceViewModel::SelectedResolutionIndex()
  {
    return _selectedResolutionIndex;
  }

  void InferenceViewModel::SelectedResolutionIndex(int32_t value)
  {
    if (value == _selectedResolutionIndex) return;

    _selectedResolutionIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedResolutionIndex"));
  }

  float InferenceViewModel::GuidanceStrength()
  {
    return _guidanceStrength;
  }

  void InferenceViewModel::GuidanceStrength(float value)
  {
    if (value == _guidanceStrength) return;

    _guidanceStrength = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"GuidanceStrength"));
  }

  float InferenceViewModel::DenoisingStrength()
  {
    return _denoisingStrength;
  }

  void InferenceViewModel::DenoisingStrength(float value)
  {
    if (value == _denoisingStrength) return;

    _denoisingStrength = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"DenoisingStrength"));
  }

  uint32_t InferenceViewModel::SamplingSteps()
  {
    return _samplingSteps;
  }

  void InferenceViewModel::SamplingSteps(uint32_t value)
  {
    if (value == _samplingSteps) return;

    _samplingSteps = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SamplingSteps"));
  }

  uint32_t InferenceViewModel::RandomSeed()
  {
    return _randomSeed;
  }

  void InferenceViewModel::RandomSeed(uint32_t value)
  {
    if (value == _randomSeed) return;

    _randomSeed = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"RandomSeed"));
  }

  bool InferenceViewModel::IsSeedFrozen()
  {
    return _isSeedFrozen;
  }

  void InferenceViewModel::IsSeedFrozen(bool value)
  {
    if (value == _isSeedFrozen) return;

    _isSeedFrozen = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsSeedFrozen"));
  }

  hstring InferenceViewModel::Status()
  {
    return _status;
  }

  void InferenceViewModel::Status(hstring const& value)
  {
    if (value == _status) return;

    _status = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"Status"));
  }

  float InferenceViewModel::Progress()
  {
    return _progress;
  }

  void InferenceViewModel::Progress(float value)
  {
    if (value == _progress) return;

    _progress = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"Progress"));
  }

  Windows::Foundation::Collections::IObservableVector<hstring> InferenceViewModel::Images()
  {
    return _images;
  }

  bool InferenceViewModel::HasImageSelected()
  {
    return _selectedImageIndex != -1;
  }

  int32_t InferenceViewModel::SelectedImageIndex()
  {
    return _selectedImageIndex;
  }

  void InferenceViewModel::SelectedImageIndex(int32_t value)
  {
    //if (value == _selectedImageIndex) return;

    _selectedImageIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedImageIndex"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"ImagePosition"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"HasImageSelected"));

    LoadImageMetadataAsync();
    RefreshOutputImageAsync();
  }

  hstring InferenceViewModel::ImagePosition()
  {
    return std::format(L"{} / {}", _selectedImageIndex + 1, _images.Size()).c_str();
  }

  Windows::Storage::StorageFile InferenceViewModel::InputImage()
  {
    return _inputImage;
  }

  fire_and_forget InferenceViewModel::InputImage(Windows::Storage::StorageFile value)
  {
    auto lifetime = get_strong();

    if (value == _inputImage) co_return;

    filesystem::path targetPath{ value.Path().c_str() };

    error_code ec;
    if (value && !filesystem::exists(targetPath, ec))
    {
      value = co_await value.CopyAsync(ApplicationData::Current().TemporaryFolder(), targetPath.filename().c_str(), NameCollisionOption::ReplaceExisting);
    }

    _inputImage = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"InputImage"));
  }

  Windows::Storage::StorageFile InferenceViewModel::OutputImage()
  {
    return _outputImage;
  }

  Windows::Foundation::Collections::IObservableVector<hstring> InferenceViewModel::Projects()
  {
    return _projects;
  }

  int32_t InferenceViewModel::SelectedProjectIndex()
  {
    return _selectedProjectIndex;
  }

  void InferenceViewModel::SelectedProjectIndex(int32_t value)
  {
    if (value == _selectedProjectIndex) return;

    _selectedProjectIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedProjectIndex"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"CanDeleteProject"));

    if (_selectedProjectIndex == -1) return;

    auto projectName = to_string(_projects.GetAt(value));
    _imageRepository->ProjectName(projectName);
    SelectedImageIndex(int32_t(_images.Size()) - 1);
  }

  bool InferenceViewModel::CanDeleteProject()
  {
    return _selectedProjectIndex != -1 && _projects.GetAt(_selectedProjectIndex) != L"scratch";
  }

  fire_and_forget InferenceViewModel::GenerateImage()
  {
    //Capture caller context
    apartment_context callerContext;
    auto lifetime = get_strong();

    //Build inference task
    auto resolution = _resolutions.GetAt(_selectedResolutionIndex);

    if (!_isSeedFrozen)
    {
      RandomSeed(_seedDistribution(_random));
    }

    StableDiffusionInferenceTask task{
      .Mode = _inferenceMode,
      .PositivePrompt = to_string(_positivePrompt.empty() ? PositivePromptPlaceholder() : _positivePrompt),
      .NegativePrompt = to_string(_negativePrompt.empty() ? NegativePromptPlaceholder() : _negativePrompt),
      .Resolution = { uint32_t(resolution.Width), uint32_t(resolution.Height) },
      .GuidanceStrength = _guidanceStrength,
      .DenoisingStrength = _denoisingStrength,
      .SamplingSteps = _samplingSteps,
      .RandomSeed = _randomSeed,
      .SafeMode = _unpaintOptions->IsSafeModeEnabled(),
      .ModelId = to_string(_models.GetAt(_selectedModelIndex))
    };

    if (_inferenceMode == InferenceMode::Modify)
    {
      if (_selectedImageIndex != -1)
      {
        task.InputImage = _inputImage ? _inputImage.Path().c_str() : _imageRepository->GetPath(to_string(_images.GetAt(_selectedImageIndex))).c_str();
      }
      else
      {
        task.Mode = InferenceMode::Create;
      }
    }

    //Run inference
    co_await resume_background();

    async_operation asyncOperation;
    asyncOperation.state_changed(no_revoke, [=](const async_operation_info& info) -> fire_and_forget {
      auto progress = info.progress;
      auto status = to_hstring(info.status_message);

      co_await callerContext;

      Progress(progress);
      Status(status);
      });

    auto results = _modelExecutor->TryRunInference(task, asyncOperation);
    
    //Update UI
    co_await callerContext;

    if (!results.empty())
    {
      Progress(0.f);
      Status(L"");

      _imageRepository->AddImage(results[0], task.ToMetadata());
      SelectedImageIndex(int32_t(_images.Size()) - 1);
    }
  }

  void InferenceViewModel::ManageModels()
  {
    _navigationService.NavigateToView(xaml_typename<ModelsView>());
  }

  void InferenceViewModel::OpenSettings()
  {
    _navigationService.NavigateToView(xaml_typename<SettingsView>());
  }

  void InferenceViewModel::CopyToClipboard()
  {
    DataPackage dataPackage{};
    dataPackage.SetStorageItems({ OutputImage() });
    Clipboard::SetContent(dataPackage);
  }

  fire_and_forget InferenceViewModel::SaveImageAs()
  {
    auto lifetime = get_strong();
    if (!HasImageSelected()) co_return;

    auto imageId = to_string(_images.GetAt(_selectedImageIndex));
    auto imagePath = _imageRepository->GetPath(imageId);
    auto buffer = try_read_file(imagePath);
    if (buffer.empty()) co_return;

    FileSavePicker fileSavePicker{};
    fileSavePicker.SuggestedFileName(imagePath.filename().c_str());
    fileSavePicker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
    fileSavePicker.FileTypeChoices().Insert(L"PNG image", single_threaded_vector<hstring>({ L".png" }));
    auto targetFile = co_await fileSavePicker.PickSaveFileAsync();
    if (!targetFile) co_return;

    co_await FileIO::WriteBytesAsync(targetFile, buffer);
  }

  fire_and_forget InferenceViewModel::DeleteImage()
  {
    auto lifetime = get_strong();
    if (!HasImageSelected()) co_return;

    ContentDialog confirmationDialog{};
    confirmationDialog.Title(box_value(L"Deleting image"));
    confirmationDialog.Content(box_value(L"Are you sure to remove this image?"));
    confirmationDialog.PrimaryButtonText(L"Yes");
    confirmationDialog.SecondaryButtonText(L"No");
    confirmationDialog.DefaultButton(ContentDialogButton::Secondary);
    auto dialogResult = co_await confirmationDialog.ShowAsync();

    if (dialogResult != ContentDialogResult::Primary) co_return;
    
    auto imageId = to_string(_images.GetAt(_selectedImageIndex));
    _imageRepository->RemoveImage(imageId);
  }

  fire_and_forget InferenceViewModel::ShowImageDirectory()
  {
    auto lifetime = get_strong();
    auto path = _imageRepository->ImageRoot();
    auto imageFolder = co_await StorageFolder::GetFolderFromPathAsync(path.c_str());
    co_await Launcher::LaunchFolderAsync(imageFolder);
  }

  fire_and_forget InferenceViewModel::CreateNewProject()
  {
    auto lifetime = get_strong();

    ContentDialog newProjectDialog{};
    newProjectDialog.Title(box_value(L"Create new project"));

    TextBox projectNameBox{};
    projectNameBox.PlaceholderText(L"Project name");

    newProjectDialog.Content(projectNameBox);
    newProjectDialog.PrimaryButtonText(L"OK");
    newProjectDialog.SecondaryButtonText(L"Cancel");
    newProjectDialog.DefaultButton(ContentDialogButton::Primary);
    auto dialogResult = co_await newProjectDialog.ShowAsync();

    auto projectName = to_string(projectNameBox.Text());
    if (dialogResult != ContentDialogResult::Primary || projectName.empty()) co_return;

    _imageRepository->ProjectName(projectName);
  }

  fire_and_forget InferenceViewModel::DeleteProject()
  {
    auto lifetime = get_strong();

    auto it = ranges::find_if(_imageRepository->Projects(), [=](const std::string& item) { return item != _imageRepository->ProjectName(); });
    if (it == _imageRepository->Projects().end()) co_return;

    ContentDialog confirmationDialog{};
    confirmationDialog.Title(box_value(L"Deleting project"));
    confirmationDialog.Content(box_value(to_hstring(std::format("Are you sure to remove project {}?", _imageRepository->ProjectName()))));
    confirmationDialog.PrimaryButtonText(L"Yes");
    confirmationDialog.SecondaryButtonText(L"No");
    confirmationDialog.DefaultButton(ContentDialogButton::Secondary);
    auto dialogResult = co_await confirmationDialog.ShowAsync();

    if (dialogResult != ContentDialogResult::Primary) co_return;

    error_code ec;
    filesystem::remove_all(_imageRepository->ImageRoot(), ec);
    _imageRepository->ProjectName(*it);
  }

  fire_and_forget InferenceViewModel::AddImage(Windows::Storage::StorageFile file)
  {
    auto lifetime = get_strong();
    apartment_context context{};

    co_await resume_background();

    TextureData texture;
    try
    {
      auto buffer = read_file(file);
      texture = TextureData::FromBuffer(buffer);
    }
    catch (...)
    { }

    co_await context;

    if (texture)
    {
      //auto fileName = filesystem::path(file.Name().c_str()).replace_extension(".png");
      _imageRepository->AddImage(texture, ImageMetadata{});
      SelectedImageIndex(int32_t(_images.Size()) - 1);
    }
  }

  void InferenceViewModel::UseCurrentImageAsInput()
  {
    InputImage(OutputImage());
  }

  void InferenceViewModel::LoadSettingsFromCurrentImage()
  {
    LoadImageMetadataAsync(true);
  }

  event_token InferenceViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void InferenceViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }

  void InferenceViewModel::OnImagesChanged(ImageRepository* sender)
  {
    update_observable_collection(sender->Images(), _images, StringMapper{});
    _propertyChanged(*this, PropertyChangedEventArgs(L"ImagePosition"));

    update_observable_collection(sender->Projects(), _projects, StringMapper{});
    
    auto it = ranges::find(sender->Projects(), sender->ProjectName());
    if (it != sender->Projects().end())
    {
      _selectedProjectIndex = int32_t(distance(sender->Projects().begin(), it));
      _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedProjectIndex"));
    }
  }
  
  fire_and_forget InferenceViewModel::LoadImageMetadataAsync(bool force)
  {
    if (_selectedImageIndex == -1 || (!force && _isSettingsLocked)) co_return;
   
    //Capture context
    apartment_context callerContext;
    auto lifetime = get_strong();
    
    //Locate image path
    auto imageId = to_string(_images.GetAt(_selectedImageIndex));
    auto imagePath = _imageRepository->GetPath(imageId);

    //Switch to background thread
    co_await resume_background();

    //Try read file into memory
    auto imageBuffer = try_read_file(imagePath);
    if (imageBuffer.empty()) co_return;

    //Try decode image and extract metadata string
    string metadataString;
    if (!TextureData::FromBuffer(imageBuffer, TextureImageFormat::Rgba8, &metadataString)) co_return;

    //Try parse metadata
    auto imageMetadata = try_parse_json<ImageMetadata>(metadataString);
    if (!imageMetadata) co_return;

    //Return to UI thread
    co_await callerContext;

    //Apply settings
    PositivePrompt(to_hstring(*imageMetadata->PositivePrompt));
    NegativePrompt(to_hstring(*imageMetadata->NegativePrompt));
    GuidanceStrength(*imageMetadata->GuidanceStrength);
    DenoisingStrength(*imageMetadata->DenoisingStrength);
    SamplingSteps(*imageMetadata->SamplingSteps);
    RandomSeed(*imageMetadata->RandomSeed);
  }

  fire_and_forget InferenceViewModel::RefreshOutputImageAsync()
  {
    StorageFile outputImage{nullptr};

    if (_selectedImageIndex != -1)
    {
      auto imagePath = _imageRepository->GetPath(to_string(_images.GetAt(_selectedImageIndex)));
      if (!imagePath.empty() && filesystem::exists(imagePath)) 
      {
        outputImage = co_await StorageFile::GetFileFromPathAsync(imagePath.c_str());
      }
    }

    _outputImage = outputImage;
    _propertyChanged(*this, PropertyChangedEventArgs(L"OutputImage"));
  }
}
