#pragma once
#include "InferenceViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel>
  {
    InferenceViewModel() = default;

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceViewModel : InferenceViewModelT<InferenceViewModel, implementation::InferenceViewModel>
  {
  };
}
