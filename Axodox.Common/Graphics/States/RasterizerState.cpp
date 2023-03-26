#include "pch.h"
#include "RasterizerState.h"
#include "Infrastructure\BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  RasterizerState::RasterizerState(const GraphicsDevice& device, RasterizerFlags flags, int depthBias, float slopeScaledDepthBias) :
    GraphicsResource(device)
  {
    CD3D11_RASTERIZER_DESC description{ CD3D11_DEFAULT() };

    if (has_flag(flags, RasterizerFlags::CullClockwise))
    {
      description.CullMode = D3D11_CULL_FRONT;
    }
    else if (has_flag(flags, RasterizerFlags::CullCounterClockwise))
    {
      description.CullMode = D3D11_CULL_BACK;
    }
    else
    {
      description.CullMode = D3D11_CULL_NONE;
    }

    if (has_flag(flags, RasterizerFlags::Wireframe))
    {
      description.FillMode = D3D11_FILL_WIREFRAME;
    }

    if (has_flag(flags, RasterizerFlags::DisableDepthClip))
    {
      description.DepthClipEnable = false;
    }

    description.DepthBias = depthBias;
    description.SlopeScaledDepthBias = slopeScaledDepthBias;
    description.MultisampleEnable = true;

    check_hresult(_device->CreateRasterizerState(&description, _state.put()));
  }

  void RasterizerState::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->RSSetState(_state.get());
  }
}