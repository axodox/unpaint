#pragma once
#include "InferenceViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel>
  {
    InferenceViewModel();

    hstring PositivePrompt();
    void PositivePrompt(hstring const& value);

    hstring NegativePrompt();
    void NegativePrompt(hstring const& value);

    Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Size> Resolutions();
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

    void GenerateImage();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    hstring _positivePrompt, _negativePrompt;
    Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Size> _resolutions;
    int32_t _selectedResolutionIndex;
    float _guidanceStrength, _denoisingStrength;
    uint32_t _samplingSteps, _randomSeed;
    bool _isSeedFrozen;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel, implementation::InferenceViewModel>
  {
  };
}
