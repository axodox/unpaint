#include "pch.h"
#include "BlendState.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  BlendState::BlendState(const GraphicsDevice& device, BlendType blendType) :
    BlendState(device, { &blendType, 1 })
  { }
  
  BlendState::BlendState(const GraphicsDevice& device, std::span<const BlendType> blendTypes) :
    GraphicsResource(device)
  {
    D3D11_BLEND_DESC description;
    zero_memory(description);

    auto count = min(ranges::size(description.RenderTarget), blendTypes.size());
    description.IndependentBlendEnable = count > 1;
    for (size_t i = 0; i < count; i++)
    {
      description.RenderTarget[i] = GetBlendDescription(blendTypes[i]);
    }

    check_hresult(_device->CreateBlendState(&description, _state.put()));
  }

  void BlendState::Bind(const DirectX::XMFLOAT4& blendFactor, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->OMSetBlendState(_state.get(), reinterpret_cast<const float*>(&blendFactor), 0xffffffff);
  }

  D3D11_RENDER_TARGET_BLEND_DESC BlendState::GetBlendDescription(BlendType type)
  {
    D3D11_RENDER_TARGET_BLEND_DESC result;
    zero_memory(result);

    switch (type)
    {
    case BlendType::Opaque:
      result.BlendEnable = false;
      result.SrcBlend = D3D11_BLEND_ONE;
      result.DestBlend = D3D11_BLEND_ZERO;
      result.BlendOp = D3D11_BLEND_OP_ADD;
      result.SrcBlendAlpha = D3D11_BLEND_ONE;
      result.DestBlendAlpha = D3D11_BLEND_ZERO;
      result.BlendOpAlpha = D3D11_BLEND_OP_ADD;
      break;
    case BlendType::Additive:
    case BlendType::Subtractive:
      result.BlendEnable = true;
      result.SrcBlend = D3D11_BLEND_ONE;
      result.DestBlend = D3D11_BLEND_ONE;
      result.BlendOp = result.BlendOpAlpha = (type == Additive ? D3D11_BLEND_OP_ADD : D3D11_BLEND_OP_REV_SUBTRACT);
      result.SrcBlendAlpha = D3D11_BLEND_ONE;
      result.DestBlendAlpha = D3D11_BLEND_ONE;
      result.BlendOpAlpha = result.BlendOp;
      break;
    case BlendType::AlphaBlend:
      result.BlendEnable = true;
      result.SrcBlend = D3D11_BLEND_SRC_ALPHA;
      result.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
      result.BlendOp = D3D11_BLEND_OP_ADD;
      result.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
      result.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
      result.BlendOpAlpha = D3D11_BLEND_OP_ADD;
      break;
    case BlendType::Minimum:
      result.BlendEnable = true;
      result.SrcBlend = D3D11_BLEND_ONE;
      result.DestBlend = D3D11_BLEND_ONE;
      result.BlendOp = D3D11_BLEND_OP_MIN;
      result.SrcBlendAlpha = D3D11_BLEND_ONE;
      result.DestBlendAlpha = D3D11_BLEND_ONE;
      result.BlendOpAlpha = D3D11_BLEND_OP_MIN;
      break;
    case BlendType::Maximum:
      result.BlendEnable = true;
      result.SrcBlend = D3D11_BLEND_ONE;
      result.DestBlend = D3D11_BLEND_ONE;
      result.BlendOp = D3D11_BLEND_OP_MAX;
      result.SrcBlendAlpha = D3D11_BLEND_ONE;
      result.DestBlendAlpha = D3D11_BLEND_ONE;
      result.BlendOpAlpha = D3D11_BLEND_OP_MAX;
      break;
    }

    result.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return result;
  }
}