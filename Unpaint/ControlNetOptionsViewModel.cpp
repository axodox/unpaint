#include "pch.h"
#include "ControlNetOptionsViewModel.h"
#include "ControlNetOptionsViewModel.g.cpp"
#include "ControlNetRepository.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  ControlNetOptionsViewModel::ControlNetOptionsViewModel() :
    _modes(single_threaded_observable_vector<ControlNetModeViewModel>()),
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _navigationService(dependencies.resolve<INavigationService>())
  {
    auto repository = dependencies.resolve<ControlNetRepository>();

    set<hstring> installedModes;
    for (auto i = 0; auto& id : repository->InstalledModes())
    {
      if (id == *_unpaintState->ControlNetMode) _selectedModeIndex = i;

      installedModes.emplace(to_hstring(id));
      i++;
    }

    for (const auto& mode : ControlNetRepository::Modes())
    {
      if(installedModes.contains(mode.Id)) _modes.Append(mode);
    }
  }

  bool ControlNetOptionsViewModel::IsAvailable()
  {
    return _modes.Size() > 1;
  }

  Windows::Foundation::Collections::IObservableVector<Unpaint::ControlNetModeViewModel> ControlNetOptionsViewModel::Modes()
  {
    return _modes;
  }

  bool ControlNetOptionsViewModel::IsEnabled()
  {
    return _unpaintState->IsControlNetEnabled;
  }

  void ControlNetOptionsViewModel::IsEnabled(bool value)
  {
    _unpaintState->IsControlNetEnabled = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsEnabled"));
  }

  int32_t ControlNetOptionsViewModel::SelectedModeIndex()
  {
    return _selectedModeIndex;
  }

  void ControlNetOptionsViewModel::SelectedModeIndex(int32_t value)
  {
    _selectedModeIndex = value;

    if(value != -1) _unpaintState->ControlNetMode = to_string(_modes.GetAt(value).Id);
  }

  void ControlNetOptionsViewModel::InstallModes()
  {
    _navigationService.OpenUri(Uri(L"unpaint://models/controlnet"));
  }

  event_token ControlNetOptionsViewModel::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ControlNetOptionsViewModel::PropertyChanged(event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }
}
