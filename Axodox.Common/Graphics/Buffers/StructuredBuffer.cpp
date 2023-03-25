#include "pch.h"
#include "StructuredBuffer.h"
#include "..\..\Infrastructure\BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  StructuredBuffer::StructuredBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, uint32_t itemSize) :
    GraphicsBuffer(device, source, BufferType::Structured, itemSize),
    _itemSize(itemSize)
  { 
    D3D11_SHADER_RESOURCE_VIEW_DESC description;
    zero_memory(description);

    description.Format = DXGI_FORMAT_UNKNOWN;
    description.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    description.Buffer.NumElements = Capacity();

    check_hresult(_device->CreateShaderResourceView(_buffer.get(), &description, _view.put()));
  }

  uint32_t StructuredBuffer::ItemSize() const
  {
    return _itemSize;
  }

  uint32_t StructuredBuffer::Capacity() const
  {
    return _capacity / _itemSize;
  }

  uint32_t StructuredBuffer::Size() const
  {
    return _size / _itemSize;
  }

  void StructuredBuffer::Bind(ShaderStage stage, uint32_t slot, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    context->BindShaderResourceView(_view.get(), stage, slot);
  }
}