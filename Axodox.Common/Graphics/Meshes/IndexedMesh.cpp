#include "pch.h"
#include "IndexedMesh.h"

namespace Axodox::Graphics
{
  void IndexedMesh::Draw(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    _indexBuffer.Bind(context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->DrawIndexed(_indexBuffer.Size(), 0, 0);
  }

  void IndexedMesh::DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    _indexBuffer.Bind(context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->DrawIndexedInstanced(_indexBuffer.Size(), instanceCount, 0, 0, 0);
  }

  uint32_t IndexedMesh::IndexCapacity() const
  {
    return _indexBuffer.Capacity();
  }
}