#pragma once
#include "Graphics/Devices/GraphicsResource.h"

namespace Axodox::Graphics
{
  enum BlendType
  {
    Opaque,
    Additive,
    Subtractive,
    AlphaBlend,
    Minimum,
    Maximum
  };

  class BlendState : public GraphicsResource
  {
  public:
    BlendState(const GraphicsDevice& device, BlendType blendType);
    BlendState(const GraphicsDevice& device, std::span<const BlendType> blendTypes);

    void Bind(const DirectX::XMFLOAT4& blendFactor = { 1.f, 1.f, 1.f, 1.f }, GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11BlendState> _state;

    static D3D11_RENDER_TARGET_BLEND_DESC GetBlendDescription(BlendType type);
  };
}