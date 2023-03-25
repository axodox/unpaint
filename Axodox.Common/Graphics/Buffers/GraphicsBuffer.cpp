#include "pch.h"
#include "GraphicsBuffer.h"
#include "..\..\Infrastructure\BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  GraphicsBuffer::GraphicsBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, BufferType type, uint32_t itemSize) :
    GraphicsResource(device)
  {
    //Load buffer source
    span<const uint8_t> buffer;
    if (holds_alternative<uint32_t>(source))
    {
      _capacity = get<uint32_t>(source);
    }
    else if (holds_alternative<span<const uint8_t>>(source))
    {
      buffer = get<span<const uint8_t>>(source);
      _capacity = uint32_t(buffer.size());
    }
    else
    {
      throw logic_error("Buffer source is not set!");
    }

    //Build buffer description
    D3D11_BUFFER_DESC description;
    D3D11_SUBRESOURCE_DATA subresourceData;
    {
      //Clear descriptions
      zero_memory(description);
      zero_memory(subresourceData);

      //Set bind flags and type related options
      switch (type)
      {
      case BufferType::Index:
        description.BindFlags = D3D11_BIND_INDEX_BUFFER;
        break;
      case BufferType::Vertex:
        description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        break;
      case BufferType::Constant:
        description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        _capacity = _capacity % 16 == 0 ? _capacity : _capacity + 16 - _capacity % 16;
        break;
      case BufferType::Structured:
        description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        description.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        description.StructureByteStride = itemSize;
        break;
      default:
        throw logic_error("Buffer type not implemented.");
      }

      //Set capacity
      description.ByteWidth = _capacity;

      //Set access
      if (buffer.empty())
      {
        _size = 0;
        description.Usage = D3D11_USAGE_DYNAMIC;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      }
      else
      {
        _size = uint32_t(buffer.size());
        description.Usage = D3D11_USAGE_IMMUTABLE;
        description.CPUAccessFlags = 0;

        subresourceData.pSysMem = buffer.data();
      }
    }

    //Create buffer
    check_hresult(device->CreateBuffer(&description, buffer.empty() ? nullptr : &subresourceData, _buffer.put()));
  }

  void GraphicsBuffer::Upload(std::span<const uint8_t> buffer, GraphicsDeviceContext* context)
  {
    //Get context
    if (!context) context = _device.ImmediateContext();

    //Calculate size
    _size = min(uint32_t(buffer.size()), _capacity);
    if (buffer.empty()) return;

    //Map subresource for update
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    check_hresult((*context)->Map(_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));

    //Copy new data
    memcpy(mappedSubresource.pData, buffer.data(), buffer.size());

    //Unmap subresource
    (*context)->Unmap(_buffer.get(), 0);
  }
}