#pragma once
#include "GraphicsBuffer.h"

namespace Axodox::Graphics
{
  class IndexBuffer : public GraphicsBuffer
  {
    friend class IndexedMesh;

  public:
    template<typename T>
    requires std::same_as<T, uint32_t> || std::same_as<T, uint16_t>
    IndexBuffer(const GraphicsDevice& device, TypedCapacityOrImmutableData<T> source) :
      IndexBuffer(device, source, sizeof(T) == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT)
    { }

    DXGI_FORMAT Format() const;
    uint32_t Capacity() const;
    uint32_t Size() const;

    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    DXGI_FORMAT _format;
    uint32_t _indexSize;

    IndexBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, DXGI_FORMAT format);
  };
}