#include "pch.h"
#include "HwndSwapChain.h"

using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  HwndSwapChain::HwndSwapChain(const GraphicsDevice& device, HWND window, SwapChainFlags flags) :
    SwapChain(device, flags)
  {
    //Create swap chain
    com_ptr<IDXGISwapChain1> swapChain;
    {
      auto swapChainDescription = CreateDescription();

      com_ptr<IDXGIFactory4> dxgiFactory;
      check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactory4>(), dxgiFactory.put_void()));

      check_hresult(dxgiFactory->CreateSwapChainForHwnd(
        device.get(),
        window,
        &swapChainDescription,
        nullptr,
        nullptr,
        swapChain.put()
      ));
    }

    //Initialize swap chain
    InitializeSwapChain(swapChain);
  }

  DirectX::XMUINT2 HwndSwapChain::GetSize() const
  {
    return { 0, 0 };
  }

  DXGI_MATRIX_3X2_F HwndSwapChain::GetTransformation() const
  {
    return { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
  }
}