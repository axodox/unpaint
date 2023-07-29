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
#include "Infrastructure/Win32.h"

using namespace Axodox::Collections;
using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;

namespace winrt::Unpaint::implementation
{
  const uint32_t InferenceViewModel::_maxSafetyStrikes = 3;

  InferenceViewModel::InferenceViewModel() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _modelExecutor(dependencies.resolve<StableDiffusionModelExecutor>()),
    _featureExtractor(dependencies.resolve<FeatureExtractionExecutor>()),
    _imageRepository(dependencies.resolve<ImageRepository>()),
    _navigationService(dependencies.resolve<INavigationService>()),
    _deviceInformation(dependencies.resolve<DeviceInformation>()),
    _isBusy(false),
    _random(uint32_t(time(nullptr))),
    _status(L""),
    _progress(0),
    _inputImage(nullptr),
    _inputMask(nullptr),
    _featureMask(nullptr),
    _inputResolution({ 0, 0 }),
    _isAutoGenerationEnabled(false),
    _safetyStrikes(0)
  { }

  ProjectViewModel InferenceViewModel::Project()
  {
    return _project;
  }

  int32_t InferenceViewModel::SelectedModeIndex()
  {
    return static_cast<int32_t>(*_unpaintState->InferenceMode);
  }

  void InferenceViewModel::SelectedModeIndex(int32_t value)
  {
    if (static_cast<InferenceMode>(value) == *_unpaintState->InferenceMode) return;

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

    if(!value) _project.LoadSettingsFromCurrentImage();
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
      auto temporaryFolder = ApplicationData::Current().TemporaryFolder();

      auto targetName = filesystem::path{ value.Name().c_str() };

      error_code ec;
      auto targetPath = filesystem::path{ temporaryFolder.Path().c_str() } / targetName;
      if (filesystem::exists(targetPath, ec))
      {
        targetName = format(L"{}-{}{}", targetName.stem().c_str(), to_hstring(make_guid()), targetName.extension().c_str()).c_str();
      }

      value = co_await value.CopyAsync(temporaryFolder, targetName.c_str(), NameCollisionOption::ReplaceExisting);
    }

    //Try parse file
    TextureData image;
    try
    {
      image = TextureData::FromBuffer(read_file(value ? value.Path().c_str() : L""));
      if (!image) co_return;
    }
    catch (...)
    {
      co_return;
    }

    //Update input image
    _inputImage = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"InputImage"));

    _featureMask = nullptr;
    _propertyChanged(*this, PropertyChangedEventArgs(L"FeatureMask"));

    //Update resolution    
    _inputResolution = BitmapSize{ image.Width, image.Height };
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

  Windows::UI::Xaml::Media::ImageSource InferenceViewModel::FeatureMask()
  {
    return _featureMask;
  }

  bool InferenceViewModel::IsAutoGenerationEnabled()
  {
    return _isAutoGenerationEnabled;
  }

  bool InferenceViewModel::HasSafetyCheckFailed()
  {
    return _hasSafetyCheckFailed;
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
    if (!_unpaintState->IsSeedFrozen)
    {
      _unpaintState->RandomSeed = _seedDistribution(_random);
    }

    StableDiffusionInferenceTask task{
      .Mode = _unpaintState->InferenceMode,
      .PositivePrompt = _unpaintState->PositivePrompt->empty() ? StableDiffusionInferenceTask::PositivePromptPlaceholder : *_unpaintState->PositivePrompt,
      .NegativePrompt = _unpaintState->NegativePrompt->empty() ? StableDiffusionInferenceTask::NegativePromptPlaceholder : *_unpaintState->NegativePrompt,
      .Resolution = { uint32_t(_unpaintState->Resolution->Width), uint32_t(_unpaintState->Resolution->Height) },
      .GuidanceStrength = _unpaintState->GuidanceStrength,
      .DenoisingStrength = _unpaintState->DenoisingStrength,
      .SamplingSteps = _unpaintState->SamplingSteps,
      .RandomSeed = _unpaintState->RandomSeed,
      .BatchSize = _unpaintState->IsBatchGenerationEnabled ? *_unpaintState->BatchSize : 1,
      .IsSafeModeEnabled = _unpaintState->IsSafeModeEnabled,
      .IsSafetyCheckerEnabled = _unpaintState->IsSafetyCheckerEnabled,
      .ModelId = _unpaintState->ModelId      
    };

    filesystem::path inputPath;
    if (*_unpaintState->InferenceMode == InferenceMode::Modify)
    {
      if (_inputImage) inputPath = _inputImage.Path().c_str();
      else if (_project.HasImageSelected()) inputPath = _project.SelectedImage().Path().c_str();
      
      if (!inputPath.empty())
      {
        task.InputImage = TextureData::FromBuffer(try_read_file(inputPath));
        task.InputMask = TextureData::FromSoftwareBitmap(_inputMask);
      }
      else
      {
        task.Mode = InferenceMode::Create;
      }

      task.ControlNetMode = _unpaintState->IsControlNetEnabled ? *_unpaintState->ControlNetMode : "";
      task.ConditioningScale = _unpaintState->ConditioningScale;
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

    //Feature extraction
    if (*_unpaintState->InferenceMode == InferenceMode::Modify && !task.ControlNetMode.empty() && task.InputImage)
    {
      auto annotatorMode = FeatureExtractionExecutor::ParseExtractionMode(task.ControlNetMode);
      if (*_unpaintState->IsAnnotatorEnabled && annotatorMode != FeatureExtractionMode::Unknown)
      {
        task.InputCondition = _featureExtractor->ExtractFeatures(task.InputImage, annotatorMode, asyncOperation).ToFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
      }
      else
      {
        task.InputCondition = TextureData(task.InputImage);
      }
    }

    //Update UI
    co_await callerContext;
    
    if (task.InputCondition)
    {
      auto featureBitmap = task.InputCondition.ToSoftwareBitmap();

      SoftwareBitmapSource featureBitmapSource;
      co_await featureBitmapSource.SetBitmapAsync(featureBitmap);
      _featureMask = featureBitmapSource;
      _propertyChanged(*this, PropertyChangedEventArgs(L"FeatureMask"));
    }
    else
    {
      _featureMask = nullptr;
    }

    co_await resume_background();

    //Stable Diffusion inference
    auto results = _modelExecutor->TryRunInference(task, asyncOperation);

    //Update UI
    co_await callerContext;

    _hasSafetyCheckFailed = false;
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

        if (!result)
        {
          _hasSafetyCheckFailed = true;
          continue;
        }

        co_await _imageRepository->AddImageAsync(result, index, task.ToMetadata());
      }

      if (_unpaintState->IsJumpingToLatestImage) _project.SelectedImageIndex(-1);
    }

    //Safety check
    _propertyChanged(*this, PropertyChangedEventArgs(L"HasSafetyCheckFailed"));
    if (_hasSafetyCheckFailed)
    {
      _safetyStrikes++;
      if (_safetyStrikes >= _maxSafetyStrikes)
      {
        _isAutoGenerationEnabled = false;
        _propertyChanged(*this, PropertyChangedEventArgs(L"IsAutoGenerationEnabled"));
      }
    }
    else
    {
      _safetyStrikes = 0;
    }

    _isBusy = false;

    //Run further generations if needed
    if (_isAutoGenerationEnabled)
    {
      GenerateImage();
    }
  }

  void InferenceViewModel::OpenSettings()
  {
    _navigationService.NavigateToView(xaml_typename<SettingsView>());
  }

  void InferenceViewModel::UseCurrentImageAsInput()
  {
    InputImage(_project.SelectedImage());
  }

  void InferenceViewModel::CopyPromptLink()
  {
    Uri result{ format(L"unpaint://inference/create?model={}&positive_prompt={}&negative_prompt={}&guidance_strength={}&sampling_steps={}&random_seed={}",
      Uri::EscapeComponent(to_hstring(*_unpaintState->ModelId)),
      Uri::EscapeComponent(to_hstring(*_unpaintState->PositivePrompt)),
      Uri::EscapeComponent(to_hstring(*_unpaintState->NegativePrompt)),
      *_unpaintState->GuidanceStrength,
      *_unpaintState->SamplingSteps,
      *_unpaintState->RandomSeed)
    };

    DataPackage dataPackage{};
    dataPackage.SetText(result.ToString());
    Clipboard::SetContent(dataPackage);
  }

  void InferenceViewModel::OpenUri(Windows::Foundation::Uri const& uri)
  {
    if(uri.Path() != L"/create") return;

    auto query = uri.QueryParsed();
    for (const auto& item : query)
    {
      if (item.Name() == L"positive_prompt")
      {
        _unpaintState->PositivePrompt = to_string(item.Value());
        _propertyChanged(*this, PropertyChangedEventArgs(L"PositivePrompt"));
      }

      if (item.Name() == L"negative_prompt")
      {
        _unpaintState->NegativePrompt = to_string(item.Value());
        _propertyChanged(*this, PropertyChangedEventArgs(L"NegativePrompt"));
      }

      if (item.Name() == L"guidance_strength")
      {
        _unpaintState->GuidanceStrength = stof(to_string(item.Value()));
        _propertyChanged(*this, PropertyChangedEventArgs(L"GuidanceStrength"));
      }

      if (item.Name() == L"sampling_steps")
      {
        _unpaintState->SamplingSteps = stoi(to_string(item.Value()));
        _propertyChanged(*this, PropertyChangedEventArgs(L"SamplingSteps"));
      }

      if (item.Name() == L"random_seed")
      {
        _unpaintState->RandomSeed = stoul(to_string(item.Value()));
        _propertyChanged(*this, PropertyChangedEventArgs(L"RandomSeed"));
      }
    }

    GenerateImage();
  }

  event_token InferenceViewModel::PropertyChanged(PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void InferenceViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
}
