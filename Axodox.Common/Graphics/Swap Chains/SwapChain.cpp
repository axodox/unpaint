#include "pch.h"
#include "SwapChain.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  const uint32_t SwapChain::_minBufferSize = 64u;

  void SwapChain::Resize()
  {
    _buffers.clear();
    auto size = GetSize();
    check_hresult(_swapChain->ResizeBuffers(2, size.x, size.y, DXGI_FORMAT_UNKNOWN, has_flag(_flags, SwapChainFlags::IsTearingAllowed) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));

    auto swapChain3 = _swapChain.as<IDXGISwapChain3>();
    auto swapChainTransform = GetTransformation();
    check_hresult(swapChain3->SetMatrixTransform(&swapChainTransform));

    check_hresult(swapChain3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709));
  }

  void SwapChain::Present()
  {
    if (has_flag(_flags, SwapChainFlags::IsTearingAllowed))
    {
      check_hresult(_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
    }
    else
    {
      check_hresult(_swapChain->Present(1, 0));
    }

    _postPresentActions.process_pending_invocations();
  }

  RenderTarget2D* SwapChain::BackBuffer()
  {
    com_ptr<ID3D11Texture2D> backBuffer;
    check_hresult(_swapChain->GetBuffer(0, guid_of<ID3D11Texture2D>(), backBuffer.put_void()));

    auto& buffer = _buffers[backBuffer.get()];
    if (!buffer)
    {
      buffer = make_unique<RenderTarget2D>(_device, backBuffer, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    }

    return buffer.get();
  }

  SwapChain::SwapChain(const GraphicsDevice& device, SwapChainFlags flags) :
    GraphicsResource(device),
    _flags(flags)
  { }

  DXGI_SWAP_CHAIN_DESC1 SwapChain::CreateDescription() const
  {
    DXGI_SWAP_CHAIN_DESC1 result;
    zero_memory(result);

    if (has_flag(_flags, SwapChainFlags::IsTearingAllowed)) result.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    result.BufferCount = 2;
    result.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    result.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    if (has_flag(_flags, SwapChainFlags::IsShaderResource)) result.BufferUsage |= DXGI_USAGE_SHADER_INPUT;
    result.SampleDesc.Count = 1;
    result.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    auto size = GetSize();
    result.Width = size.x;
    result.Height = size.y;

    return result;
  }

  void SwapChain::InitializeSwapChain(const winrt::com_ptr<IDXGISwapChain>& swapChain)
  {
    _swapChain = swapChain;

    //Set transform
    auto swapChain2 = swapChain.as<IDXGISwapChain2>();
    auto swapChainTransform = GetTransformation();
    check_hresult(swapChain2->SetMatrixTransform(&swapChainTransform));
  }
}