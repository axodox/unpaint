#pragma once
#include "ProjectViewModel.g.h"
#include "UnpaintState.h"
#include "ImageRepository.h"
#include "Threading/RecursionLock.h"

namespace winrt::Unpaint::implementation
{
  struct ProjectViewModel : ProjectViewModelT<ProjectViewModel>
  {
    ProjectViewModel();

#pragma region Projects
    winrt::Windows::Foundation::Collections::IObservableVector<hstring> Projects();

    int32_t SelectedProjectIndex();
    void SelectedProjectIndex(int32_t value);

    bool CanDeleteProject();

    fire_and_forget CreateNewProject();
    fire_and_forget OpenProjectDirectory();
    fire_and_forget DeleteProject();
#pragma endregion

#pragma region Images
    winrt::Windows::Foundation::Collections::IObservableVector<hstring> Images();

    int32_t SelectedImageIndex();
    void SelectedImageIndex(int32_t value);

    bool HasImageSelected();
    winrt::Windows::Storage::StorageFile SelectedImage();
    hstring ImagePosition();

    fire_and_forget AddImage(winrt::Windows::Storage::StorageFile const& file);
    fire_and_forget LoadSettingsFromCurrentImage();
    void CopyToClipboard();
    fire_and_forget SaveImageAs();
    fire_and_forget DeleteImage();
#pragma endregion

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<ImageRepository> _imageRepository;
    Axodox::Threading::recursion_counter _recursionBlock;

#pragma region Projects
    Windows::Foundation::Collections::IObservableVector<hstring> _projects;
    int32_t _selectedProjectIndex;
#pragma endregion

#pragma region Images
    Windows::Foundation::Collections::IObservableVector<hstring> _images;
    int32_t _selectedImageIndex;
    Windows::Storage::StorageFile _selectedImage;
    Axodox::Infrastructure::event_subscription _imagesChangedSubscription;

    fire_and_forget RefreshSelectedImageAsync();
    void OnImagesChanged(ImageRepository* sender);
#pragma endregion

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ProjectViewModel : ProjectViewModelT<ProjectViewModel, implementation::ProjectViewModel>
  {
  };
}