#include "pch.h"
#include "DeviceInformation.h"

using namespace winrt::Windows::System::Profile;

namespace winrt::Unpaint
{
  DeviceInformation::DeviceInformation() :
    _isDeviceXbox(AnalyticsInfo::VersionInfo().DeviceFamily() == L"Windows.Xbox")
  { }

  bool DeviceInformation::IsDeviceXbox() const
  {
    return _isDeviceXbox;
  }
}