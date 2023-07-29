#include "pch.h"
#include "SettingsViewModel.h"
#include "SettingsViewModel.g.cpp"
#include "Infrastructure/WinRtDependencies.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Axodox::Graphics;
using namespace std;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation;

namespace winrt::Unpaint::implementation
{
  SettingsViewModel::SettingsViewModel() :
    _navigationService(dependencies.resolve<INavigationService>()),
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _adapters(single_threaded_observable_vector<AdapterViewModel>())
  { 
    //Populate adapter list
    auto adapters = GraphicsDevice::Adapters();
    for (auto& adapter : adapters)
    {
      _adapters.Append(AdapterViewModel{
        .Name = adapter.Index == 0u ? format(L"Default ({})", adapter.Name.c_str()) : adapter.Name.c_str(),
        .Index = adapter.Index
        });
    }
  }

  bool SettingsViewModel::AreUnsafeOptionsEnabled()
  {
#ifdef NDEBUG
    //Sorry mates I do not trust you this much...
    return false;
#else
    return true;
#endif
  }

  bool SettingsViewModel::IsSafeModeEnabled()
  {
    return _unpaintState->IsSafeModeEnabled;
  }

  void SettingsViewModel::IsSafeModeEnabled(bool value)
  {
    _unpaintState->IsSafeModeEnabled = value;
  }

  bool SettingsViewModel::IsSafetyCheckerEnabled()
  {
    return _unpaintState->IsSafetyCheckerEnabled;
  }

  void SettingsViewModel::IsSafetyCheckerEnabled(bool value)
  {
    _unpaintState->IsSafetyCheckerEnabled = value;
  }

  bool SettingsViewModel::IsDenoiserPinned()
  {
    return _unpaintState->IsDenoiserPinned;
  }

  void SettingsViewModel::IsDenoiserPinned(bool value)
  {
    _unpaintState->IsDenoiserPinned = value;
  }

  hstring SettingsViewModel::Version()
  {
    auto version = Package::Current().Id().Version();
    return format(L"{}.{}.{}.{}", version.Major, version.Minor, version.Build, version.Revision);
  }

  Windows::Foundation::Collections::IObservableVector<AdapterViewModel> SettingsViewModel::Adapters()
  {
    return _adapters;
  }

  int32_t SettingsViewModel::SelectedAdapterIndex()
  {
    return _unpaintState->AdapterIndex;
  }

  void SettingsViewModel::SelectedAdapterIndex(int32_t value)
  {
    _unpaintState->AdapterIndex = value;
  }
  
  void SettingsViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<InferenceView>());
  }
}
