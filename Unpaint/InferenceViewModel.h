#pragma once
#include "InferenceViewModel.g.h"
#include "ModelRepository.h"
#include "StableDiffusionModelExecutor.h"
#include "ImageRepository.h"
#include "UnpaintOptions.h"
#include "UnpaintState.h"
#include "Infrastructure/Events.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel>
  {
    InferenceViewModel();

    int32_t SelectedModeIndex();
    void SelectedModeIndex(int32_t value);

    bool IsSettingsLocked();
    void IsSettingsLocked(bool value);

    bool IsJumpingToLatestImage();
    void IsJumpingToLatestImage(bool value);

    hstring PositivePromptPlaceholder();
    hstring PositivePrompt();
    fire_and_forget PositivePrompt(hstring const& value);
    int32_t AvailablePositiveTokenCount();

    hstring NegativePromptPlaceholder();
    hstring NegativePrompt();
    fire_and_forget NegativePrompt(hstring const& value);
    int32_t AvailableNegativeTokenCount();

    Windows::Foundation::Collections::IObservableVector<hstring> Models();
    int32_t SelectedModelIndex();
    void SelectedModelIndex(int32_t value);

    Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> Resolutions();
    int32_t SelectedResolutionIndex();
    void SelectedResolutionIndex(int32_t value);

    bool IsBatchGenerationEnabled();
    void IsBatchGenerationEnabled(bool value);

    uint32_t BatchSize();
    void BatchSize(uint32_t value);

    float GuidanceStrength();
    void GuidanceStrength(float value);

    float DenoisingStrength();
    void DenoisingStrength(float value);
    
    uint32_t SamplingSteps();
    void SamplingSteps(uint32_t value);

    uint32_t RandomSeed();
    void RandomSeed(uint32_t value);

    bool IsSeedFrozen();
    void IsSeedFrozen(bool value);

    hstring Status();
    void Status(hstring const& value);

    float Progress();
    void Progress(float value);

    Windows::Foundation::Collections::IObservableVector<hstring> Images();
    bool HasImageSelected();
    int32_t SelectedImageIndex();
    void SelectedImageIndex(int32_t value);
    hstring ImagePosition();

    Windows::Storage::StorageFile InputImage();
    fire_and_forget InputImage(Windows::Storage::StorageFile value);
    Windows::Graphics::Imaging::BitmapSize InputResolution();

    Windows::Graphics::Imaging::SoftwareBitmap InputMask();
    void InputMask(Windows::Graphics::Imaging::SoftwareBitmap const& value);

    Windows::Storage::StorageFile OutputImage();

    Windows::Foundation::Collections::IObservableVector<hstring> Projects();
    int32_t SelectedProjectIndex();
    void SelectedProjectIndex(int32_t value);
    bool CanDeleteProject();

    bool IsAutoGenerationEnabled();
    bool HasSafetyCheckFailed();

    fire_and_forget GenerateImage();
    void ManageModels();
    void OpenSettings();

    void CopyToClipboard();
    fire_and_forget SaveImageAs();
    fire_and_forget DeleteImage();
    fire_and_forget ShowImageDirectory();
    fire_and_forget CreateNewProject();
    fire_and_forget DeleteProject();
    fire_and_forget AddImage(Windows::Storage::StorageFile file);
    void UseCurrentImageAsInput();
    void LoadSettingsFromCurrentImage();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    static const uint32_t _maxSafetyStrikes;

    INavigationService _navigationService;
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;
    std::shared_ptr<StableDiffusionModelExecutor> _modelExecutor;
    std::shared_ptr<ImageRepository> _imageRepository;

    std::minstd_rand _random;
    std::uniform_int_distribution<uint32_t> _seedDistribution;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    bool _isBusy;

    int32_t _availablePositiveTokenCount, _availableNegativeTokenCount;
    
    Windows::Foundation::Collections::IObservableVector<hstring> _models;
    int32_t _selectedModelIndex;

    Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> _resolutions;
    int32_t _selectedResolutionIndex;
   
    hstring _status;
    float _progress;

    Axodox::Infrastructure::event_subscription _imagesChangedSubscription;
    Windows::Foundation::Collections::IObservableVector<hstring> _images;
    int32_t _selectedImageIndex;
    Windows::Storage::StorageFile _outputImage, _inputImage;
    Windows::Graphics::Imaging::BitmapSize _inputResolution;
    Windows::Graphics::Imaging::SoftwareBitmap _inputMask;

    bool _isAutoGenerationEnabled;
    bool _hasSafetyCheckFailed;
    uint32_t _safetyStrikes;

    Windows::Foundation::Collections::IObservableVector<hstring> _projects;
    int32_t _selectedProjectIndex;

    void OnImagesChanged(ImageRepository* sender);

    fire_and_forget LoadImageMetadataAsync(bool force = false);
    fire_and_forget RefreshOutputImageAsync();

    Windows::Foundation::IAsyncOperation<int32_t> ValidatePromptAsync(hstring prompt, bool isSafeModeEnabled);
    fire_and_forget UpdatePositivePromptAsync();
    fire_and_forget UpdateNegativePromptAsync();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel, implementation::InferenceViewModel>
  {
  };
}
