#pragma once
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class HwndSwapChain : public SwapChain
  {
  public:
    HwndSwapChain(const GraphicsDevice& device, HWND window, SwapChainFlags flags = SwapChainFlags::Default);

  protected:
    virtual DirectX::XMUINT2 GetSize() const override;
    virtual DXGI_MATRIX_3X2_F GetTransformation() const override;
  };
}