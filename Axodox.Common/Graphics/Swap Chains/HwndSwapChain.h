#pragma once
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class HwndSwapChain : public SwapChain
  {
  public:
    HwndSwapChain(const GraphicsDevice& device, HWND window, SwapChainFlags flags = SwapChainFlags::Default);
  };
}