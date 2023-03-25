#pragma once
#include "GraphicsBuffer.h"

namespace Axodox::Graphics
{
  class ConstantBuffer : public GraphicsBuffer
  {
  public:
    template<typename T>
    explicit ConstantBuffer(const GraphicsDevice& device) :
      ConstantBuffer(device, sizeof(T))
    { }

  private:
    ConstantBuffer(const GraphicsDevice& device, uint32_t size);

    void Bind(ShaderStage stage, uint32_t slot = 0u, GraphicsDeviceContext* context = nullptr);
  };
}