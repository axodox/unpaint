#pragma once
#include "..\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  class SamplerState : public GraphicsResource
  {
  public:
    SamplerState(const GraphicsDevice& device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, const DirectX::XMFLOAT4& borderColor = DirectX::XMFLOAT4{ 0.f, 0.f, 0.f, 0.f });

    void Bind(ShaderStage stage, uint32_t slot = 0, GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11SamplerState> _state;
  };
}