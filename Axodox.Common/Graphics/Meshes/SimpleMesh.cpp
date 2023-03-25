#include "pch.h"
#include "SimpleMesh.h"

namespace Axodox::Graphics
{
  void SimpleMesh::Draw(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->Draw(_vertexBuffer.Size(), 0);
  }

  void SimpleMesh::DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    _vertexBuffer.Bind(0u, context);
    (*context)->IASetPrimitiveTopology(_topology);
    (*context)->DrawInstanced(_vertexBuffer.Size(), instanceCount, 0, 0);
  }

  VertexDefition SimpleMesh::Defition() const
  {
    return _vertexBuffer.Definition();
  }
  
  uint32_t SimpleMesh::VertexCapacity() const
  {
    return _vertexBuffer.Capacity();
  }
}