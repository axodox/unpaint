#include "pch.h"
#include "InferenceOptionsViewModel.h"
#include "InferenceOptionsViewModel.g.cpp"

using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning::Imaging::StableDiffusion::Schedulers;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  InferenceOptionsViewModel::InferenceOptionsViewModel() :
    _navigationService(dependencies.resolve<INavigationService>()),
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _deviceInformation(dependencies.resolve<DeviceInformation>()),
    _models(single_threaded_observable_vector<ModelViewModel>()),
    _schedulers(single_threaded_observable_vector<hstring>()),
    _resolutions(single_threaded_observable_vector<SizeInt32>()),
    _selectedResolutionIndex(1),
    _selectedModelIndex(0),
    _onStateChangedSubscription(_unpaintState->StateChanged(event_handler{ this, &InferenceOptionsViewModel::OnStateChanged }))
  {
    //Initialize models
    OnModelChanged();

    //Initialize schedulers
    _schedulers.Append(L"Euler A");
    _schedulers.Append(L"DPM++ 2M Karras");

    //Initialize resolutions
    _resolutions.Append(SizeInt32{ 1024, 1024 });
    _resolutions.Append(SizeInt32{ 768, 768 });
    _resolutions.Append(SizeInt32{ 512, 512 });
    OnResolutionChanged();    
  }

  winrt::Windows::Foundation::Collections::IObservableVector<winrt::Unpaint::ModelViewModel> InferenceOptionsViewModel::Models()
  {
    return _models;
  }

  int32_t InferenceOptionsViewModel::SelectedModelIndex()
  {
    return _selectedModelIndex;
  }

  void InferenceOptionsViewModel::SelectedModelIndex(int32_t value)
  {
    if (value == _selectedModelIndex) return;

    _selectedModelIndex = value;
    if (value != -1) _unpaintState->ModelId = to_string(_models.GetAt(value).Id);
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedModelIndex"));
  }

  winrt::Windows::Foundation::Collections::IObservableVector<winrt::hstring> InferenceOptionsViewModel::Schedulers()
  {
    return _schedulers;
  }

  int32_t InferenceOptionsViewModel::SelectedSchedulerIndex()
  {
    return int32_t(*_unpaintState->Scheduler);
  }

  void InferenceOptionsViewModel::SelectedSchedulerIndex(int32_t value)
  {
    if (value == SelectedSchedulerIndex()) return;

    _unpaintState->Scheduler = StableDiffusionSchedulerKind(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedSchedulerIndex"));
  }

  winrt::Windows::Foundation::Collections::IObservableVector<winrt::Windows::Graphics::SizeInt32> InferenceOptionsViewModel::Resolutions()
  {
    return _resolutions;
  }

  int32_t InferenceOptionsViewModel::SelectedResolutionIndex()
  {
    return _selectedResolutionIndex;
  }

  void InferenceOptionsViewModel::SelectedResolutionIndex(int32_t value)
  {
    if (value == _selectedResolutionIndex) return;

    _selectedResolutionIndex = value;
    if (value != -1) _unpaintState->Resolution = _resolutions.GetAt(value);
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedResolutionIndex"));
  }

  bool InferenceOptionsViewModel::IsBatchGenerationEnabled()
  {
    return _unpaintState->IsBatchGenerationEnabled;
  }

  void InferenceOptionsViewModel::IsBatchGenerationEnabled(bool value)
  {
    if (value == _unpaintState->IsBatchGenerationEnabled) return;

    _unpaintState->IsBatchGenerationEnabled = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsBatchGenerationEnabled"));
  }

  uint32_t InferenceOptionsViewModel::BatchSize()
  {
    return _unpaintState->BatchSize;
  }

  void InferenceOptionsViewModel::BatchSize(uint32_t value)
  {
    if (value == _unpaintState->BatchSize) return;

    _unpaintState->BatchSize = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"BatchSize"));
  }

  float InferenceOptionsViewModel::GuidanceStrength()
  {
    return _unpaintState->GuidanceStrength;
  }

  void InferenceOptionsViewModel::GuidanceStrength(float value)
  {
    if (value == _unpaintState->GuidanceStrength) return;

    _unpaintState->GuidanceStrength = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"GuidanceStrength"));
  }

  uint32_t InferenceOptionsViewModel::SamplingSteps()
  {
    return _unpaintState->SamplingSteps;
  }

  void InferenceOptionsViewModel::SamplingSteps(uint32_t value)
  {
    if (value == _unpaintState->SamplingSteps) return;

    _unpaintState->SamplingSteps = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SamplingSteps"));
  }

  uint32_t InferenceOptionsViewModel::RandomSeed()
  {
    return _unpaintState->RandomSeed;
  }

  void InferenceOptionsViewModel::RandomSeed(uint32_t value)
  {
    if (value == _unpaintState->RandomSeed) return;

    _unpaintState->RandomSeed = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"RandomSeed"));
  }

  bool InferenceOptionsViewModel::IsSeedFrozen()
  {
    return _unpaintState->IsSeedFrozen;
  }

  void InferenceOptionsViewModel::IsSeedFrozen(bool value)
  {
    if (value == _unpaintState->IsSeedFrozen) return;

    _unpaintState->IsSeedFrozen = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsSeedFrozen"));
  }

  void InferenceOptionsViewModel::ManageModels()
  {
    _navigationService.NavigateToView(xaml_typename<ModelsView>());
  }

  winrt::event_token InferenceOptionsViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void InferenceOptionsViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }

  void InferenceOptionsViewModel::OnStateChanged(OptionPropertyBase* property)
  {
    if (property == &_unpaintState->Resolution) OnResolutionChanged();
    if (property == &_unpaintState->IsBatchGenerationEnabled) _propertyChanged(*this, PropertyChangedEventArgs(L"IsBatchGenerationEnabled"));
    if (property == &_unpaintState->BatchSize) _propertyChanged(*this, PropertyChangedEventArgs(L"BatchSize"));
    if (property == &_unpaintState->GuidanceStrength) _propertyChanged(*this, PropertyChangedEventArgs(L"GuidanceStrength"));
    if (property == &_unpaintState->SamplingSteps) _propertyChanged(*this, PropertyChangedEventArgs(L"SamplingSteps"));
    if (property == &_unpaintState->RandomSeed) _propertyChanged(*this, PropertyChangedEventArgs(L"RandomSeed"));
    if (property == &_unpaintState->IsSeedFrozen) _propertyChanged(*this, PropertyChangedEventArgs(L"IsSeedFrozen"));
  }

  void InferenceOptionsViewModel::OnModelChanged()
  {
    auto modelId = *_unpaintState->ModelId;
    for (auto i = 0; auto & model : _modelRepository->Models())
    {
      if (model.Id == modelId) _selectedModelIndex = i;
      _models.Append(model);

      i++;
    }
  }

  void InferenceOptionsViewModel::OnResolutionChanged()
  {
    for (auto i = 0; auto resolution : _resolutions)
    {
      if (resolution == _unpaintState->Resolution)
      {
        SelectedResolutionIndex(i);
        break;
      }

      i++;
    }
  }
}
