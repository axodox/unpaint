#pragma once
#include "pch.h"
#include "winrt/Unpaint.h"

namespace winrt::Unpaint
{
  class ControlNetRepository
  {
  public:
    static std::vector<ControlNetModeViewModel> Modes();
  };
}