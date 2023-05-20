#pragma once
#include "InferenceViewModel.g.h"
#include "ModelRepository.h"
#include "StableDiffusionModelExecutor.h"
#include "ImageRepository.h"
#include "UnpaintOptions.h"
#include "Infrastructure/Events.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel>
  {
    InferenceViewModel();

    int32_t SelectedModeIndex();
    void SelectedModeIndex(int32_t value);

    hstring PositivePromptPlaceholder();
    hstring PositivePrompt();
    void PositivePrompt(hstring const& value);

    hstring NegativePromptPlaceholder();
    hstring NegativePrompt();
    void NegativePrompt(hstring const& value);

    Windows::Foundation::Collections::IObservableVector<hstring> Models();
    int32_t SelectedModelIndex();
    void SelectedModelIndex(int32_t value);

    Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> Resolutions();
    int32_t SelectedResolutionIndex();
    void SelectedResolutionIndex(int32_t value);

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

    Windows::UI::Xaml::Media::ImageSource OutputImage();
    void OutputImage(Windows::UI::Xaml::Media::ImageSource const& value);

    Windows::Foundation::Collections::IObservableVector<hstring> Projects();
    int32_t SelectedProjectIndex();
    void SelectedProjectIndex(int32_t value);
    bool CanDeleteProject();

    fire_and_forget GenerateImage();
    void ManageModels();
    void OpenSettings();

    fire_and_forget CopyToClipboard();
    fire_and_forget SaveImageAs();
    fire_and_forget DeleteImage();
    fire_and_forget ShowImageDirectory();
    fire_and_forget CreateNewProject();
    fire_and_forget DeleteProject();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    INavigationService _navigationService;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;
    std::shared_ptr<StableDiffusionModelExecutor> _modelExecutor;
    std::shared_ptr<ImageRepository> _imageRepository;

    std::minstd_rand _random;
    std::uniform_int_distribution<uint32_t> _seedDistribution;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    InferenceMode _inferenceMode;

    hstring _positivePrompt, _negativePrompt;
    
    Windows::Foundation::Collections::IObservableVector<hstring> _models;
    int32_t _selectedModelIndex;

    Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> _resolutions;
    int32_t _selectedResolutionIndex;
    
    float _guidanceStrength, _denoisingStrength;
    uint32_t _samplingSteps, _randomSeed;
    bool _isSeedFrozen;

    hstring _status;
    float _progress;

    Axodox::Infrastructure::event_subscription _imagesChangedSubscription;
    Windows::Foundation::Collections::IObservableVector<hstring> _images;
    int32_t _selectedImageIndex;
    Windows::UI::Xaml::Media::ImageSource _outputImage;

    Windows::Foundation::Collections::IObservableVector<hstring> _projects;
    int32_t _selectedProjectIndex;

    void OnImagesChanged(ImageRepository* sender);

    fire_and_forget LoadImageMetadataAsync();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel, implementation::InferenceViewModel>
  {
  };
}
