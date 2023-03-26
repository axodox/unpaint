#include "pch.h"
#include "IndexBuffer.h"
#include "Graphics/Helpers.h"

namespace Axodox::Graphics
{
  IndexBuffer::IndexBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, DXGI_FORMAT format) :
    GraphicsBuffer(device, source, BufferType::Index),
    _format(format),
    _indexSize(uint32_t(BitsPerPixel(format) / 8))
  { }

  DXGI_FORMAT IndexBuffer::Format() const
  {
    return _format;
  }

  uint32_t IndexBuffer::Capacity() const
  {
    return _capacity / _indexSize;
  }

  uint32_t IndexBuffer::Size() const
  {
    return _size / _indexSize;
  }
  
  void IndexBuffer::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->IASetIndexBuffer(_buffer.get(), _format, 0);
  }
}