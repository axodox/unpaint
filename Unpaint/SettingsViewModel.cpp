#include "pch.h"
#include "SettingsViewModel.h"
#include "SettingsViewModel.g.cpp"
#include "Infrastructure/WinRtDependencies.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;

namespace winrt::Unpaint::implementation
{
  SettingsViewModel::SettingsViewModel() :
    _navigationService(dependencies.resolve<INavigationService>()),
    _unpaintOptions(dependencies.resolve<UnpaintOptions>())
  { }

  bool SettingsViewModel::AreUnsafeOptionsEnabled()
  {
#ifdef NDEBUG
    return false;
#else
    return true;
#endif // NDEBUG
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
  
  void SettingsViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<InferenceView>());
  }
}
