#include "pch.h"
#include "VertexBuffer.h"

namespace Axodox::Graphics
{
  VertexBuffer::VertexBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, VertexDefition vertexDefition, uint32_t itemSize) :
    GraphicsBuffer(device, source, BufferType::Vertex),
    _vertexDefition(vertexDefition),
    _itemSize(itemSize)
  { }

  VertexDefition VertexBuffer::Definition() const
  {
    return _vertexDefition;
  }

  uint32_t VertexBuffer::ItemSize() const
  {
    return _itemSize;
  }

  uint32_t VertexBuffer::Capacity() const
  {
    return _capacity / _itemSize;
  }

  uint32_t VertexBuffer::Size() const
  {
    return _size / _itemSize;
  }

  void VertexBuffer::Bind(uint32_t slot, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    auto buffer = _buffer.get();
    auto stride = _itemSize;
    auto offset = 0u;
    (*context)->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);
  }
}