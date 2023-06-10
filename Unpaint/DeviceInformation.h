#pragma once
#include "pch.h"

namespace winrt::Unpaint
{
  class DeviceInformation
  {
  public:
    DeviceInformation();

    bool IsDeviceXbox() const;

  private:
    bool _isDeviceXbox;
  };
}