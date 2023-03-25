#include "pch.h"
#include "SamplerState.h"

using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  SamplerState::SamplerState(const GraphicsDevice& device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, const DirectX::XMFLOAT4& borderColor) :
    GraphicsResource(device)
  {
    CD3D11_SAMPLER_DESC description{
      filter,
      addressMode,
      addressMode,
      addressMode,
      0.f,
      filter == D3D11_FILTER_ANISOTROPIC ? 8u : 1u,
      D3D11_COMPARISON_NEVER,
      reinterpret_cast<const float*>(&borderColor),
      0.f,
      D3D11_FLOAT32_MAX
    };

    check_hresult(_device->CreateSamplerState(&description, _state.put()));
  }

  void SamplerState::Bind(ShaderStage stage, uint32_t slot, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    auto state = _state.get();
    switch (stage)
    {
    case ShaderStage::Vertex:
      (*context)->VSSetSamplers(slot, 1, &state);
      break;
    case ShaderStage::Hull:
      (*context)->HSSetSamplers(slot, 1, &state);
      break;
    case ShaderStage::Domain:
      (*context)->DSSetSamplers(slot, 1, &state);
      break;
    case ShaderStage::Geometry:
      (*context)->GSSetSamplers(slot, 1, &state);
      break;
    case ShaderStage::Pixel:
      (*context)->PSSetSamplers(slot, 1, &state);
      break;
    case ShaderStage::Compute:
      (*context)->CSSetSamplers(slot, 1, &state);
      break;
    default:
      throw logic_error("Shader stage not implemented!");
    }
  }
}