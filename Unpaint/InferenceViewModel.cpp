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
using namespace Axodox::MachineLearning;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Imaging;
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
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _unpaintOptions(dependencies.resolve<UnpaintOptions>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _modelExecutor(dependencies.resolve<StableDiffusionModelExecutor>()),
    _imageRepository(dependencies.resolve<ImageRepository>()),
    _navigationService(dependencies.resolve<INavigationService>()),
    _isBusy(false),
    _availablePositiveTokenCount(int32_t(TextTokenizer::MaxTokenCount)),
    _availableNegativeTokenCount(int32_t(TextTokenizer::MaxTokenCount)),
    _models(single_threaded_observable_vector<hstring>()),
    _resolutions(single_threaded_observable_vector<SizeInt32>()),
    _selectedResolutionIndex(1),
    _random(uint32_t(time(nullptr))),
    _status(L""),
    _progress(0),
    _images(single_threaded_observable_vector<hstring>()),
    _projects(single_threaded_observable_vector<hstring>()),
    _outputImage(nullptr),
    _inputImage(nullptr),
    _inputMask(nullptr),
    _inputResolution({ 0, 0 }),
    _isAutoGenerationEnabled(false),
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
    return static_cast<int32_t>(_unpaintState->InferenceMode);
  }

  void InferenceViewModel::SelectedModeIndex(int32_t value)
  {
    if (static_cast<InferenceMode>(value) == _unpaintState->InferenceMode) return;

    _unpaintState->InferenceMode = static_cast<InferenceMode>(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedModeIndex"));
  }

  bool InferenceViewModel::IsSettingsLocked()
  {
    return _unpaintState->IsSettingsLocked;
  }

  void InferenceViewModel::IsSettingsLocked(bool value)
  {
    if (value == _unpaintState->IsSettingsLocked) return;

    _unpaintState->IsSettingsLocked = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsSettingsLocked"));

    LoadImageMetadataAsync();
  }

  bool InferenceViewModel::IsJumpingToLatestImage()
  {
    return _unpaintState->IsJumpingToLatestImage;
  }

  void InferenceViewModel::IsJumpingToLatestImage(bool value)
  {
    if (value == _unpaintState->IsJumpingToLatestImage) return;

    _unpaintState->IsJumpingToLatestImage = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsJumpingToLatestImage"));
  }

  hstring InferenceViewModel::PositivePromptPlaceholder()
  {
    return L"an empty canvas standing in a painter's workshop";
  }

  hstring InferenceViewModel::PositivePrompt()
  {
    return _unpaintState->PositivePrompt;
  }

  fire_and_forget InferenceViewModel::PositivePrompt(hstring const& value)
  {
    if (value == _unpaintState->PositivePrompt) co_return;

    _unpaintState->PositivePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"PositivePrompt"));

    auto lifetime = get_strong();
    _availablePositiveTokenCount = co_await ValidatePromptAsync(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"AvailablePositiveTokenCount"));
  }

  int32_t InferenceViewModel::AvailablePositiveTokenCount()
  {
    return _availablePositiveTokenCount;
  }

  hstring InferenceViewModel::NegativePromptPlaceholder()
  {
    return L"blurry, render";
  }

  hstring InferenceViewModel::NegativePrompt()
  {
    return _unpaintState->NegativePrompt;
  }

  fire_and_forget InferenceViewModel::NegativePrompt(hstring const& value)
  {
    if (value == _unpaintState->NegativePrompt) co_return;

    _unpaintState->NegativePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"NegativePrompt"));

    auto lifetime = get_strong();
    _availableNegativeTokenCount = co_await ValidatePromptAsync(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"AvailableNegativeTokenCount"));
  }

  int32_t InferenceViewModel::AvailableNegativeTokenCount()
  {
    return _availableNegativeTokenCount;
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

  bool InferenceViewModel::IsBatchGenerationEnabled()
  {
    return _unpaintState->IsBatchGenerationEnabled;
  }

  void InferenceViewModel::IsBatchGenerationEnabled(bool value)
  {
    if (value == _unpaintState->IsBatchGenerationEnabled) return;

    _unpaintState->IsBatchGenerationEnabled = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsBatchGenerationEnabled"));
  }

  uint32_t InferenceViewModel::BatchSize()
  {
    return _unpaintState->BatchSize;
  }

  void InferenceViewModel::BatchSize(uint32_t value)
  {
    if (value == _unpaintState->BatchSize) return;

    _unpaintState->BatchSize = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"BatchSize"));
  }

  float InferenceViewModel::GuidanceStrength()
  {
    return _unpaintState->GuidanceStrength;
  }

  void InferenceViewModel::GuidanceStrength(float value)
  {
    if (value == _unpaintState->GuidanceStrength) return;

    _unpaintState->GuidanceStrength = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"GuidanceStrength"));
  }

  float InferenceViewModel::DenoisingStrength()
  {
    return _unpaintState->DenoisingStrength;
  }

  void InferenceViewModel::DenoisingStrength(float value)
  {
    if (value == _unpaintState->DenoisingStrength) return;

    _unpaintState->DenoisingStrength = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"DenoisingStrength"));
  }

  uint32_t InferenceViewModel::SamplingSteps()
  {
    return _unpaintState->SamplingSteps;
  }

  void InferenceViewModel::SamplingSteps(uint32_t value)
  {
    if (value == _unpaintState->SamplingSteps) return;

    _unpaintState->SamplingSteps = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SamplingSteps"));
  }

  uint32_t InferenceViewModel::RandomSeed()
  {
    return _unpaintState->RandomSeed;
  }

  void InferenceViewModel::RandomSeed(uint32_t value)
  {
    if (value == _unpaintState->RandomSeed) return;

    _unpaintState->RandomSeed = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"RandomSeed"));
  }

  bool InferenceViewModel::IsSeedFrozen()
  {
    return _unpaintState->IsSeedFrozen;
  }

  void InferenceViewModel::IsSeedFrozen(bool value)
  {
    if (value == _unpaintState->IsSeedFrozen) return;

    _unpaintState->IsSeedFrozen = value;
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

    //Copy to temp for later use
    if (value)
    {
      filesystem::path targetPath{ value.Path().c_str() };

      error_code ec;
      if (value && !filesystem::exists(targetPath, ec))
      {
        value = co_await value.CopyAsync(ApplicationData::Current().TemporaryFolder(), value.Name().c_str(), NameCollisionOption::ReplaceExisting);
      }
    }

    //Update input image
    _inputImage = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"InputImage"));

    //Update resolution
    auto image = TextureData::FromBuffer(try_read_file(value ? value.Path().c_str() : L""));
    _inputResolution = image ? BitmapSize{ image.Width, image.Height } : BitmapSize{ 0, 0 };
    _propertyChanged(*this, PropertyChangedEventArgs(L"InputResolution"));
  }

  Windows::Graphics::Imaging::BitmapSize InferenceViewModel::InputResolution()
  {
    return _inputResolution;
  }

  Windows::Graphics::Imaging::SoftwareBitmap InferenceViewModel::InputMask()
  {
    return _inputMask;
  }

  void InferenceViewModel::InputMask(Windows::Graphics::Imaging::SoftwareBitmap const& value)
  {
    if (value == _inputMask) return;

    _inputMask = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"InputMask"));
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

  bool InferenceViewModel::IsAutoGenerationEnabled()
  {
    return _isAutoGenerationEnabled;
  }

  fire_and_forget InferenceViewModel::GenerateImage()
  {
    if (_isBusy)
    {
      _isAutoGenerationEnabled = !_isAutoGenerationEnabled;
      _propertyChanged(*this, PropertyChangedEventArgs(L"IsAutoGenerationEnabled"));
      co_return;
    }
    _isBusy = true;

    //Capture caller context
    apartment_context callerContext;
    auto lifetime = get_strong();

    //Build inference task
    auto resolution = _resolutions.GetAt(_selectedResolutionIndex);

    if (!_unpaintState->IsSeedFrozen)
    {
      RandomSeed(_seedDistribution(_random));
    }

    StableDiffusionInferenceTask task{
      .Mode = _unpaintState->InferenceMode,
      .PositivePrompt = to_string(_unpaintState->PositivePrompt.empty() ? PositivePromptPlaceholder() : _unpaintState->PositivePrompt),
      .NegativePrompt = to_string(_unpaintState->NegativePrompt.empty() ? NegativePromptPlaceholder() : _unpaintState->NegativePrompt),
      .Resolution = { uint32_t(resolution.Width), uint32_t(resolution.Height) },
      .GuidanceStrength = _unpaintState->GuidanceStrength,
      .DenoisingStrength = _unpaintState->DenoisingStrength,
      .SamplingSteps = _unpaintState->SamplingSteps,
      .RandomSeed = _unpaintState->RandomSeed,
      .BatchSize = _unpaintState->IsBatchGenerationEnabled ? _unpaintState->BatchSize : 1,
      .SafeMode = _unpaintOptions->IsSafeModeEnabled(),
      .ModelId = to_string(_models.GetAt(_selectedModelIndex))
    };

    if (_unpaintState->InferenceMode == InferenceMode::Modify)
    {
      if (_selectedImageIndex != -1)
      {
        task.InputImage = _inputImage ? _inputImage.Path().c_str() : _imageRepository->GetPath(to_string(_images.GetAt(_selectedImageIndex))).c_str();
        task.InputMask = TextureData::FromSoftwareBitmap(_inputMask);
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

      for (int32_t i = 0; auto & result : results)
      {
        optional<int32_t> index;
        if (results.size() > 1)
        {
          index = i++;
        }

        co_await _imageRepository->AddImageAsync(result, index, task.ToMetadata());
      }

      if (_unpaintState->IsJumpingToLatestImage) SelectedImageIndex(int32_t(_images.Size()) - 1);
    }

    _isBusy = false;

    //Run further generations if needed
    if (_isAutoGenerationEnabled)
    {
      GenerateImage();
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
    {
    }

    co_await context;

    if (texture)
    {
      co_await _imageRepository->AddImageAsync(texture, nullopt, ImageMetadata{});
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
    if (_selectedImageIndex == -1 || (!force && _unpaintState->IsSettingsLocked)) co_return;

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
    StorageFile outputImage{ nullptr };

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

  Windows::Foundation::IAsyncOperation<int32_t> InferenceViewModel::ValidatePromptAsync(hstring prompt)
  {
    apartment_context callingContext{};

    auto modelId = to_string(_models.GetAt(_selectedModelIndex));

    auto lifetime = get_strong();
    co_await resume_background();

    auto result = _modelExecutor->ValidatePrompt(modelId, to_string(prompt));

    co_await callingContext;

    co_return result;
  }
}
