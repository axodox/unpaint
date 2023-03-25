#pragma once
#include "GraphicsBuffer.h"

namespace Axodox::Graphics
{
  class StructuredBuffer : public GraphicsBuffer
  {
  public:
    template<typename T>
    StructuredBuffer(const GraphicsDevice& device, TypedCapacityOrImmutableData<T> source) :
      StructuredBuffer(device, source, sizeof(T))
    { }

    uint32_t ItemSize() const;
    uint32_t Capacity() const;
    uint32_t Size() const;
    
    void Bind(ShaderStage stage, uint32_t slot = 0u, GraphicsDeviceContext* context = nullptr);

  private:
    uint32_t _itemSize;
    winrt::com_ptr<ID3D11ShaderResourceView> _view;

    StructuredBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, uint32_t itemSize);
  };
}