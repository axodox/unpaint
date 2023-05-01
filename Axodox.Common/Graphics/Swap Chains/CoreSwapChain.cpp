#include "pch.h"
#ifdef USE_UWP_UI
#include "CoreSwapChain.h"
#include "Threading/UwpThreading.h"

using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  CoreSwapChain::CoreSwapChain(const GraphicsDevice& device, const CoreWindow& window, SwapChainFlags flags) :
    SwapChain(device, flags),
    _window(window)
  {
    //Create swap chain
    com_ptr<IDXGISwapChain1> swapChain;
    {
      auto swapChainDescription = CreateDescription();

      com_ptr<IDXGIFactory4> dxgiFactory;
      check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactory4>(), dxgiFactory.put_void()));

      check_hresult(dxgiFactory->CreateSwapChainForCoreWindow(
        device.get(),
        get_unknown(_window),
        &swapChainDescription,
        nullptr,
        swapChain.put()
      ));
    }

    //Subscribe to events
    _sizeChangedRevoker = window.SizeChanged(auto_revoke, [this](auto&, auto&) {
      _postPresentActions.invoke_async([this]() { Resize(); });
      });

    //Initialize swap chain
    InitializeSwapChain(swapChain);
  }
}
#endif