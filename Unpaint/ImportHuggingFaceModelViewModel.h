#pragma once
#include "ImportHuggingFaceModelViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ImportHuggingFaceModelViewModel : ImportHuggingFaceModelViewModelT<ImportHuggingFaceModelViewModel>
  {
    hstring ModelId();
    fire_and_forget ModelId(hstring const& value);

    bool IsValid();
    hstring Status();

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    hstring _modelId, _status;
    bool _isValid = false;

    Axodox::Web::HuggingFaceClient _huggingFaceClient;
  };
}
namespace winrt::Unpaint::factory_implementation
{
  struct ImportHuggingFaceModelViewModel : ImportHuggingFaceModelViewModelT<ImportHuggingFaceModelViewModel, implementation::ImportHuggingFaceModelViewModel>
  {
  };
}
