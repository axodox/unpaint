#include "pch.h"
#include "InferenceViewModel.h"
#include "InferenceViewModel.g.cpp"

using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  event_token InferenceViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void InferenceViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
}
