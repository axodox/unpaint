#pragma once
#include "..\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  class GeometryShader : public GraphicsResource
  {
  public:
    GeometryShader(const GraphicsDevice& device, std::span<const uint8_t> buffer);

    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11GeometryShader> _shader;
  };
}