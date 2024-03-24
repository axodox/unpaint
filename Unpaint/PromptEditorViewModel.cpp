#include "pch.h"
#include "PromptEditorViewModel.h"
#include "PromptEditorViewModel.g.cpp"

using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning::Imaging::StableDiffusion;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  PromptEditorViewModel::PromptEditorViewModel() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _modelExecutor(dependencies.resolve<StableDiffusionModelExecutor>()),
    _availablePositiveTokenCount(int32_t(TextTokenizer::MaxTokenCount)),
    _availableNegativeTokenCount(int32_t(TextTokenizer::MaxTokenCount)),
    _onStateChangedSubscription(_unpaintState->StateChanged(event_handler{ this, &PromptEditorViewModel::OnStateChanged }))
  {
    UpdatePositivePromptAsync();
    UpdateNegativePromptAsync();
  }

  hstring PromptEditorViewModel::PositivePrompt()
  {
    return to_hstring(*_unpaintState->PositivePrompt);
  }

  fire_and_forget PromptEditorViewModel::PositivePrompt(hstring const& hvalue)
  {
    auto value = to_string(hvalue);
    if (value == *_unpaintState->PositivePrompt) co_return;

    _unpaintState->PositivePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"PositivePrompt"));

    UpdatePositivePromptAsync();
  }

  hstring PromptEditorViewModel::PositivePromptPlaceholder()
  {
    return to_hstring(StableDiffusionInferenceTask::PositivePromptPlaceholder);
  }

  int32_t PromptEditorViewModel::AvailablePositiveTokenCount()
  {
    return _availablePositiveTokenCount;
  }

  hstring PromptEditorViewModel::NegativePrompt()
  {
    return to_hstring(*_unpaintState->NegativePrompt);
  }

  fire_and_forget PromptEditorViewModel::NegativePrompt(hstring const& hvalue)
  {
    auto value = to_string(hvalue);
    if (value == *_unpaintState->NegativePrompt) co_return;

    _unpaintState->NegativePrompt = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"NegativePrompt"));

    UpdateNegativePromptAsync();
  }

  hstring PromptEditorViewModel::NegativePromptPlaceholder()
  {
    return to_hstring(StableDiffusionInferenceTask::NegativePromptPlaceholder);
  }

  int32_t PromptEditorViewModel::AvailableNegativeTokenCount()
  {
    return _availableNegativeTokenCount;
  }

  winrt::event_token PromptEditorViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void PromptEditorViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }

  fire_and_forget PromptEditorViewModel::UpdatePositivePromptAsync()
  {
    auto lifetime = get_strong();
    _availablePositiveTokenCount = co_await ValidatePromptAsync(_unpaintState->PositivePrompt, false);
    _propertyChanged(*this, PropertyChangedEventArgs(L"AvailablePositiveTokenCount"));
  }

  fire_and_forget PromptEditorViewModel::UpdateNegativePromptAsync()
  {
    auto lifetime = get_strong();
    _availableNegativeTokenCount = co_await ValidatePromptAsync(_unpaintState->NegativePrompt, _unpaintState->IsSafeModeEnabled);
    _propertyChanged(*this, PropertyChangedEventArgs(L"AvailableNegativeTokenCount"));
  }

  Windows::Foundation::IAsyncOperation<int32_t> PromptEditorViewModel::ValidatePromptAsync(std::string prompt, bool isSafeModeEnabled)
  {
    apartment_context callingContext{};

    auto modelId = *_unpaintState->ModelId;

    auto lifetime = get_strong();
    co_await resume_background();

    auto result = _modelExecutor->ValidatePrompt(modelId, prompt, isSafeModeEnabled);

    co_await callingContext;

    co_return result;
  }

  void PromptEditorViewModel::OnStateChanged(OptionPropertyBase* property)
  {
    if (property == &_unpaintState->PositivePrompt)
    {
      _propertyChanged(*this, PropertyChangedEventArgs(L"PositivePrompt"));
      UpdatePositivePromptAsync();
    }

    if (property == &_unpaintState->NegativePrompt)
    {
      _propertyChanged(*this, PropertyChangedEventArgs(L"NegativePrompt"));
      UpdateNegativePromptAsync();
    }
  }
}
