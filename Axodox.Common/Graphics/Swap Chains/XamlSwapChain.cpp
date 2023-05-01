#include "pch.h"
#ifdef USE_UWP_UI
#include "XamlSwapChain.h"
#include "Threading/UwpThreading.h"

using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  XamlSwapChain::XamlSwapChain(const GraphicsDevice& device, const SwapChainPanel& panel, SwapChainFlags flags) :
    SwapChain(device, flags),
    _panel(panel)
  {
    //Create swap chain
    com_ptr<IDXGISwapChain1> swapChain;
    {
      auto swapChainDescription = CreateDescription();

      com_ptr<IDXGIFactory4> dxgiFactory;
      check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactory4>(), dxgiFactory.put_void()));

      check_hresult(dxgiFactory->CreateSwapChainForComposition(
        device.get(),
        &swapChainDescription,
        nullptr,
        swapChain.put()
      ));
    }

    //Subscribe to events
    {
      auto nativePanel = panel.as<ISwapChainPanelNative>();
      nativePanel->SetSwapChain(swapChain.get());

      _sizeChangedRevoker = panel.SizeChanged(auto_revoke, [this](auto&, auto&) {
        _postPresentActions.invoke_async([this]() { Resize(); });
        });

      _compositionScaleChangedRevoker = panel.CompositionScaleChanged(auto_revoke, [this](auto&, auto&) {
        _postPresentActions.invoke_async([this]() { Resize(); });
        });
    }

    //Initialize swap chain
    InitializeSwapChain(swapChain);
  }

  DirectX::XMUINT2 XamlSwapChain::GetSize() const
  {
    XMUINT2 result;

    send_or_post(_panel.Dispatcher(), [&]() {
      result = XMUINT2{
        max((uint32_t)(_panel.ActualWidth() * _panel.CompositionScaleX()), _minBufferSize),
        max((uint32_t)(_panel.ActualHeight() * _panel.CompositionScaleY()), _minBufferSize)
      };
    });

    return result;
  }

  DXGI_MATRIX_3X2_F XamlSwapChain::GetTransformation() const
  {
    DXGI_MATRIX_3X2_F result;

    send_or_post(_panel.Dispatcher(), [&]() {
      result = DXGI_MATRIX_3X2_F{
        1.0f / _panel.CompositionScaleX(), 0.f, 0.f,
        1.0f / _panel.CompositionScaleY(), 0.f, 0.f
      };
    });

    return result;
  }
}
#endif