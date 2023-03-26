#pragma once
#include "Graphics\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  enum class RasterizerFlags : uint32_t
  {
    None = 0,
    Default = 0,
    CullNone = 0,
    CullClockwise = 1,
    CullCounterClockwise = 2,
    Wireframe = 4,
    DisableDepthClip = 8
  };

  class RasterizerState : public GraphicsResource
  {
  public:
    RasterizerState(const GraphicsDevice& device, RasterizerFlags flags, int depthBias = 0, float slopeScaledDepthBias = 0.f);

    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11RasterizerState> _state;
  };
}