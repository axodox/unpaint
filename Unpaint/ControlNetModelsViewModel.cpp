#include "pch.h"
#include "ControlNetModelsViewModel.h"
#include "ControlNetModelsViewModel.g.cpp"
#include "ControlNetRepository.h"

using namespace std;
using namespace winrt;

namespace winrt::Unpaint::implementation
{
  ControlNetModelsViewModel::ControlNetModelsViewModel() :
    _modes(single_threaded_observable_vector<ControlNetModeViewModel>())
  { 
    for (const auto& mode : ControlNetRepository::Modes())
    {
      _modes.Append(mode);
    }
  }

  Windows::Foundation::Collections::IObservableVector<ControlNetModeViewModel> ControlNetModelsViewModel::Modes()
  {
    return _modes;
  }

  bool ControlNetModelsViewModel::IsSelected(hstring const& modeId)
  {
    return _selectedModes.contains(modeId);
  }

  void ControlNetModelsViewModel::IsSelected(hstring const& modeId, bool value)
  {
    if (value)
    {
      _selectedModes.emplace(modeId);
    }
    else
    {
      _selectedModes.erase(modeId);
    }
  }

  Windows::Foundation::Collections::IVector<hstring> ControlNetModelsViewModel::SelectedModes()
  {
    return single_threaded_vector(vector<hstring>(_selectedModes.begin(), _selectedModes.end()));
  }

  void ControlNetModelsViewModel::SelectedModes(Windows::Foundation::Collections::IVector<hstring> const& value)
  {
    _selectedModes.clear();
    for (const auto& mode : value)
    {
      _selectedModes.emplace(mode);
    }
  }

  winrt::event_token ControlNetModelsViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ControlNetModelsViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }
}
