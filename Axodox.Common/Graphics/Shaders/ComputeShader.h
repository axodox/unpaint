#pragma once
#include "Graphics\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  class ComputeShader : public GraphicsResource
  {
  public:
    ComputeShader(const GraphicsDevice& device, std::span<const uint8_t> buffer);

    void Run(DirectX::XMUINT3 groupCount, GraphicsDeviceContext* context = nullptr);

  private:
    winrt::com_ptr<ID3D11ComputeShader> _shader;
  };
}