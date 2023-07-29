#pragma once
#include "PromptEditorViewModel.g.h"
#include "UnpaintState.h"
#include "StableDiffusionModelExecutor.h"

namespace winrt::Unpaint::implementation
{
  struct PromptEditorViewModel : PromptEditorViewModelT<PromptEditorViewModel>
  {
    PromptEditorViewModel();

    hstring PositivePrompt();
    fire_and_forget PositivePrompt(hstring const& value);

    hstring PositivePromptPlaceholder();
    int32_t AvailablePositiveTokenCount();

    hstring NegativePrompt();
    fire_and_forget NegativePrompt(hstring const& value);

    hstring NegativePromptPlaceholder();
    int32_t AvailableNegativeTokenCount();

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<StableDiffusionModelExecutor> _modelExecutor;
    int32_t _availablePositiveTokenCount, _availableNegativeTokenCount;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    Axodox::Infrastructure::event_subscription _onStateChangedSubscription;

    fire_and_forget UpdatePositivePromptAsync();
    fire_and_forget UpdateNegativePromptAsync();
    Windows::Foundation::IAsyncOperation<int32_t> ValidatePromptAsync(std::string prompt, bool isSafeModeEnabled);

    void OnStateChanged(OptionPropertyBase* property);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct PromptEditorViewModel : PromptEditorViewModelT<PromptEditorViewModel, implementation::PromptEditorViewModel>
  {
  };
}
