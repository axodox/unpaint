#include "pch.h"
#include "InferenceViewModel.h"
#include "InferenceViewModel.g.cpp"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  InferenceViewModel::InferenceViewModel() :
    _guidanceStrength(7.f),
    _denoisingStrength(0.2f),
    _resolutions(single_threaded_observable_vector<Size>()),
    _selectedResolutionIndex(1),
    _samplingSteps(15),
    _randomSeed(0),
    _isSeedFrozen(false)
  { 
    _resolutions.Append(Size{ 1024, 1024 });
    _resolutions.Append(Size{ 768, 768 });
    _resolutions.Append(Size{ 512, 512 });
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

  Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Size> InferenceViewModel::Resolutions()
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

  void InferenceViewModel::GenerateImage()
  {

  }

  event_token InferenceViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void InferenceViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
}
