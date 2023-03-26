#pragma once
#include "Graphics/Devices/GraphicsResource.h"
#include "Graphics/Textures/RenderTarget2D.h"
#include "Threading/ManualDispatcher.h"

namespace Axodox::Graphics
{
  enum class SwapChainFlags
  {
    Default,
    IsShaderResource,
    IsTearingAllowed //For VRR (Variable Refresh Rate) support
  };

  class SwapChain : public GraphicsResource
  {
  public:
    void Resize();
    void Present();

    RenderTarget2D* BackBuffer();

  protected:
    static const uint32_t _minBufferSize;
    Threading::manual_dispatcher _postPresentActions;

    SwapChain(const GraphicsDevice& device, SwapChainFlags flags);

    DXGI_SWAP_CHAIN_DESC1 CreateDescription() const;

    virtual DirectX::XMUINT2 GetSize() const = 0;
    virtual DXGI_MATRIX_3X2_F GetTransformation() const = 0;

    void InitializeSwapChain(const winrt::com_ptr<IDXGISwapChain>& swapChain);

  private:
    std::unordered_map<void*, std::unique_ptr<RenderTarget2D>> _buffers;
    winrt::com_ptr<IDXGISwapChain> _swapChain;
    SwapChainFlags _flags;
  };
}