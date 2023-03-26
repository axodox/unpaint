#pragma once
#ifdef UWP_UI
#include "SwapChain.h"

namespace Axodox::Graphics
{
  class CoreSwapChain : public SwapChain
  {
    typedef winrt::Windows::UI::Core::CoreWindow CoreWindow;

  public:
    CoreSwapChain(const GraphicsDevice& device, const CoreWindow& window, SwapChainFlags flags = SwapChainFlags::Default);

  private:
    CoreWindow _window;
    CoreWindow::SizeChanged_revoker _sizeChangedRevoker;
  };
}
#endif