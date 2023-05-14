#include "pch.h"
#include "InferenceViewModel.h"
#include "InferenceViewModel.g.cpp"
#include "Infrastructure/DependencyContainer.h"
#include "Threading/AsyncOperation.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::UI::Xaml::Data;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;

namespace winrt::Unpaint::implementation
{
  InferenceViewModel::InferenceViewModel() :
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _modelExecutor(dependencies.resolve<StableDiffusionModelExecutor>()),
    _guidanceStrength(7.f),
    _denoisingStrength(0.2f),
    _models(single_threaded_observable_vector<hstring>()),
    _resolutions(single_threaded_observable_vector<SizeInt32>()),
    _selectedResolutionIndex(1),
    _samplingSteps(15),
    _randomSeed(0),
    _isSeedFrozen(false),
    _status(L""),
    _progress(0),
    _outputImage(nullptr)
  {
    for (auto& model : _modelRepository->Models())
    {
      _models.Append(to_hstring(model));
    }

    _resolutions.Append(SizeInt32{ 1024, 1024 });
    _resolutions.Append(SizeInt32{ 768, 768 });
    _resolutions.Append(SizeInt32{ 512, 512 });
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

  Windows::UI::Xaml::Media::ImageSource InferenceViewModel::OutputImage()
  {
    return _outputImage;
  }

  void InferenceViewModel::OutputImage(Windows::UI::Xaml::Media::ImageSource const& value)
  {
    if (value == _outputImage) return;

    _outputImage = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"OutputImage"));
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
      .PositivePrompt = to_string(_positivePrompt.empty() ? PositivePromptPlaceholder() : _positivePrompt),
      .NegativePrompt = to_string(_negativePrompt.empty() ? NegativePromptPlaceholder() : _negativePrompt),
      .Resolution = { uint32_t(resolution.Width), uint32_t(resolution.Height) },
      .GuidanceStrength = _guidanceStrength,
      .SamplingSteps = _samplingSteps,
      .RandomSeed = _randomSeed,
      .ModelId = to_string(_models.GetAt(_selectedModelIndex))
    };

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

    auto result = _modelExecutor->TryRunInference(task, asyncOperation);

    //Update UI
    co_await callerContext;

    if (result)
    {
      auto textureData = result.ToTextureData();
      auto softwareBitmap = textureData[0].ToSoftwareBitmap();

      SoftwareBitmapSource outputBitmap;
      co_await outputBitmap.SetBitmapAsync(softwareBitmap);

      OutputImage(outputBitmap);

      Progress(0.f);
      Status(L"");
    }
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
