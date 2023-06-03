#include "pch.h"
#include "SettingsViewModel.h"
#include "SettingsViewModel.g.cpp"
#include "Infrastructure/WinRtDependencies.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace std;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation;

namespace winrt::Unpaint::implementation
{
  SettingsViewModel::SettingsViewModel() :
    _navigationService(dependencies.resolve<INavigationService>()),
    _unpaintOptions(dependencies.resolve<UnpaintOptions>())
  { }

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
    return _unpaintOptions->IsSafeModeEnabled();
  }

  void SettingsViewModel::IsSafeModeEnabled(bool value)
  {
    _unpaintOptions->IsSafeModeEnabled(value);
  }

  bool SettingsViewModel::IsSafetyCheckerEnabled()
  {
    return _unpaintOptions->IsSafetyCheckerEnabled();
  }

  void SettingsViewModel::IsSafetyCheckerEnabled(bool value)
  {
    _unpaintOptions->IsSafetyCheckerEnabled(value);
  }

  bool SettingsViewModel::IsDenoiserPinned()
  {
    return _unpaintOptions->IsDenoiserPinned();
  }

  void SettingsViewModel::IsDenoiserPinned(bool value)
  {
    _unpaintOptions->IsDenoiserPinned(value);
  }

  hstring SettingsViewModel::Version()
  {
    auto version = Package::Current().Id().Version();
    return format(L"{}.{}.{}.{}", version.Major, version.Minor, version.Build, version.Revision);
  }

  Windows::Foundation::Uri SettingsViewModel::SelectedModelUri()
  {
    return Uri(to_hstring("https://huggingface.co/" + _unpaintOptions->ModelId()));
  }
  
  void SettingsViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<InferenceView>());
  }
}
