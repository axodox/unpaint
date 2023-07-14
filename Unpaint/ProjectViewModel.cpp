#include "pch.h"
#include "ProjectViewModel.h"
#include "ProjectViewModel.g.cpp"
#include "Collections/ObservableExtensions.h"
#include "Storage/FileIO.h"
#include "Storage/UwpStorage.h"
#include "StringMapper.h"

using namespace Axodox::Collections;
using namespace Axodox::Infrastructure;
using namespace Axodox::Graphics;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace std;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  ProjectViewModel::ProjectViewModel() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _imageRepository(dependencies.resolve<ImageRepository>()),

    _projects(single_threaded_observable_vector<hstring>()),
    _selectedProjectIndex(-1),

    _images(single_threaded_observable_vector<hstring>()),
    _selectedImageIndex(-1),
    _selectedImage(nullptr),

    _imagesChangedSubscription(_imageRepository->ImagesChanged(event_handler{ this, &ProjectViewModel::OnImagesChanged }))
  {
    //Initialize projects
    _imageRepository->Refresh();
    for (auto i = 0; const auto & project : _projects)
    {
      if (to_string(project) == *_unpaintState->Project)
      {
        SelectedProjectIndex(i);
        break;
      }

      i++;
    }

    //Initialize images
    for (auto i = 0; const auto & image : _images)
    {
      if (to_string(image) == *_unpaintState->Image)
      {
        SelectedImageIndex(i);
        break;
      }

      i++;
    }

    if (_selectedImageIndex == -1) SelectedImageIndex(-1);
  }

#pragma region Projects
  winrt::Windows::Foundation::Collections::IObservableVector<hstring> ProjectViewModel::Projects()
  {
    return _projects;
  }

  int32_t ProjectViewModel::SelectedProjectIndex()
  {
    return _selectedProjectIndex;
  }

  void ProjectViewModel::SelectedProjectIndex(int32_t value)
  {
    if (value == -1) value = int32_t(_images.Size()) - 1;
    if (value == _selectedProjectIndex) return;

    _selectedProjectIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedProjectIndex"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"CanDeleteProject"));

    if (_selectedProjectIndex == -1) return;

    auto projectName = to_string(_projects.GetAt(value));
    _unpaintState->Project = projectName;
    _imageRepository->ProjectName(projectName);
    SelectedImageIndex(int32_t(_images.Size()) - 1);
  }

  bool ProjectViewModel::CanDeleteProject()
  {
    return _selectedProjectIndex != -1 && _projects.GetAt(_selectedProjectIndex) != L"scratch";
  }

  fire_and_forget ProjectViewModel::CreateNewProject()
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

  fire_and_forget ProjectViewModel::OpenProjectDirectory()
  {
    auto lifetime = get_strong();
    auto path = _imageRepository->ImageRoot();
    auto imageFolder = co_await StorageFolder::GetFolderFromPathAsync(path.c_str());
    co_await Launcher::LaunchFolderAsync(imageFolder);
  }

  fire_and_forget ProjectViewModel::DeleteProject()
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
#pragma endregion

#pragma region Images
  winrt::Windows::Foundation::Collections::IObservableVector<hstring> ProjectViewModel::Images()
  {
    return _images;
  }

  int32_t ProjectViewModel::SelectedImageIndex()
  {
    return _selectedImageIndex;
  }

  void ProjectViewModel::SelectedImageIndex(int32_t value)
  {
    if (value == -1) value = int32_t(_images.Size()) - 1;
    if (value == _selectedImageIndex) return;

    _selectedImageIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedImageIndex"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"ImagePosition"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"HasImageSelected"));

    if (value != -1) _unpaintState->Image = to_string(_images.GetAt(value));

    if (!_unpaintState->IsSettingsLocked) LoadSettingsFromCurrentImage();
    RefreshSelectedImageAsync();
  }

  bool ProjectViewModel::HasImageSelected()
  {
    return _selectedImageIndex != -1;
  }

  winrt::Windows::Storage::StorageFile ProjectViewModel::SelectedImage()
  {
    return _selectedImage;
  }

  hstring ProjectViewModel::ImagePosition()
  {
    return std::format(L"{} / {}", _selectedImageIndex + 1, _images.Size()).c_str();
  }

  fire_and_forget ProjectViewModel::AddImage(winrt::Windows::Storage::StorageFile file)
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
      co_await _imageRepository->AddImageAsync(texture, nullopt, ImageMetadata{});
      SelectedImageIndex(int32_t(_images.Size()) - 1);
    }
  }

  fire_and_forget ProjectViewModel::LoadSettingsFromCurrentImage()
  {
    if (_selectedImageIndex == -1) co_return;

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
    _unpaintState->PositivePrompt = *imageMetadata->PositivePrompt;
    _unpaintState->NegativePrompt = *imageMetadata->NegativePrompt;
    _unpaintState->GuidanceStrength = *imageMetadata->GuidanceStrength;
    _unpaintState->DenoisingStrength = *imageMetadata->DenoisingStrength;
    _unpaintState->SamplingSteps = *imageMetadata->SamplingSteps;
    _unpaintState->RandomSeed = *imageMetadata->RandomSeed;
  }

  void ProjectViewModel::CopyToClipboard()
  {
    DataPackage dataPackage{};
    dataPackage.SetStorageItems({ SelectedImage() });
    Clipboard::SetContent(dataPackage);
  }

  fire_and_forget ProjectViewModel::SaveImageAs()
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

  fire_and_forget ProjectViewModel::DeleteImage()
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

  fire_and_forget ProjectViewModel::RefreshSelectedImageAsync()
  {
    auto lifetime = get_strong();
    StorageFile selectedImage{ nullptr };

    if (_selectedImageIndex != -1)
    {
      auto imagePath = _imageRepository->GetPath(to_string(_images.GetAt(_selectedImageIndex)));
      if (!imagePath.empty() && filesystem::exists(imagePath))
      {
        selectedImage = co_await StorageFile::GetFileFromPathAsync(imagePath.c_str());
      }
    }

    _selectedImage = selectedImage;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedImage"));
  }

  void ProjectViewModel::OnImagesChanged(ImageRepository* sender)
  {
    if (_recursionBlock.is_locked()) return;
    auto lock = _recursionBlock.lock();

    update_observable_collection(sender->Images(), _images, StringMapper{});
    _propertyChanged(*this, PropertyChangedEventArgs(L"ImagePosition"));

    string projectName{ sender->ProjectName() };
    update_observable_collection(sender->Projects(), _projects, StringMapper{});

    auto it = ranges::find(sender->Projects(), projectName);
    if (it != sender->Projects().end())
    {
      _selectedProjectIndex = int32_t(distance(sender->Projects().begin(), it));
      _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedProjectIndex"));
      _propertyChanged(*this, PropertyChangedEventArgs(L"CanDeleteProject"));
    }
  }
#pragma endregion

  winrt::event_token ProjectViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ProjectViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }
}
