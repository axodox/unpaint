#include "pch.h"
#include "DepthStencilState.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  DepthStencilState::DepthStencilState(const GraphicsDevice& device, bool writeDepth, D3D11_COMPARISON_FUNC comparison) :
    GraphicsResource(device)
  {
    CD3D11_DEPTH_STENCIL_DESC description{ CD3D11_DEFAULT() };
    
    if (!writeDepth && comparison == D3D11_COMPARISON_ALWAYS)
    {
      description.DepthEnable = false;
    }
    else
    {
      description.DepthWriteMask = writeDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
      description.DepthFunc = comparison;
    }

    check_hresult(_device->CreateDepthStencilState(&description, _state.put()));
  }

  void DepthStencilState::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->OMSetDepthStencilState(_state.get(), 0);
  }
}