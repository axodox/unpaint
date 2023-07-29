#pragma once
#include "InferenceOptionsViewModel.g.h"
#include "UnpaintState.h"
#include "ModelRepository.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceOptionsViewModel : InferenceOptionsViewModelT<InferenceOptionsViewModel>
  {
    InferenceOptionsViewModel();

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Unpaint::ModelViewModel> Models();

    int32_t SelectedModelIndex();
    void SelectedModelIndex(int32_t value);

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Windows::Graphics::SizeInt32> Resolutions();
    
    int32_t SelectedResolutionIndex();
    void SelectedResolutionIndex(int32_t value);

    bool IsBatchGenerationEnabled();
    void IsBatchGenerationEnabled(bool value);

    uint32_t BatchSize();
    void BatchSize(uint32_t value);

    float GuidanceStrength();
    void GuidanceStrength(float value);

    uint32_t SamplingSteps();
    void SamplingSteps(uint32_t value);

    uint32_t RandomSeed();
    void RandomSeed(uint32_t value);

    bool IsSeedFrozen();
    void IsSeedFrozen(bool value);

    void ManageModels();

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    INavigationService _navigationService;
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<ModelRepository> _modelRepository;
    std::shared_ptr<DeviceInformation> _deviceInformation;

    Windows::Foundation::Collections::IObservableVector<ModelViewModel> _models;
    int32_t _selectedModelIndex;

    Windows::Foundation::Collections::IObservableVector<Windows::Graphics::SizeInt32> _resolutions;
    int32_t _selectedResolutionIndex;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    Axodox::Infrastructure::event_subscription _onStateChangedSubscription;

    void OnStateChanged(OptionPropertyBase* property);
    void OnModelChanged();
    void OnResolutionChanged();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceOptionsViewModel : InferenceOptionsViewModelT<InferenceOptionsViewModel, implementation::InferenceOptionsViewModel>
  {
  };
}
