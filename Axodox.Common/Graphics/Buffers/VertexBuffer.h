#pragma once
#include "GraphicsBuffer.h"
#include "Graphics\Meshes\VertexDefinitions.h"

namespace Axodox::Graphics
{
  class VertexBuffer : public GraphicsBuffer
  {
    friend class SimpleMesh;

  public:
    template<typename T>
    VertexBuffer(const GraphicsDevice& device, TypedCapacityOrImmutableData<T> source) :
      VertexBuffer(device, source, T::Defition(), sizeof(T))
    { }
    
    VertexDefition Definition() const;

    uint32_t ItemSize() const;
    uint32_t Capacity() const;
    uint32_t Size() const;

    void Bind(uint32_t slot = 0, GraphicsDeviceContext* context = nullptr);

  private:
    uint32_t _itemSize;
    VertexDefition _vertexDefition;

    VertexBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, VertexDefition vertexDefition, uint32_t itemSize);
  };
}