#pragma once
#include "..\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  typedef std::variant<uint32_t, std::span<const uint8_t>> CapacityOrImmutableData;
  
  template<typename T>
  struct TypedCapacityOrImmutableData : public CapacityOrImmutableData
  {
    typedef T ItemType;

    TypedCapacityOrImmutableData() = default;

    TypedCapacityOrImmutableData(uint32_t capacity) :
      CapacityOrImmutableData(capacity * sizeof(T))
    { }

    TypedCapacityOrImmutableData(std::span<const T> immutableData) :
      CapacityOrImmutableData(std::span<const uint8_t>{ immutableData.data(), immutableData.size() * sizeof(T) })
    { }
  };

  enum class BufferType
  {
    Index,
    Vertex,
    Constant,
    Structured
  };

  class GraphicsBuffer : public GraphicsResource
  {
  public:
    GraphicsBuffer(const GraphicsDevice& device, CapacityOrImmutableData source, BufferType type, uint32_t itemSize = 0u);

    void Upload(std::span<const uint8_t> buffer, GraphicsDeviceContext* context = nullptr);

    template<typename T>
    void Upload(std::span<const T> items, GraphicsDeviceContext* context = nullptr)
    {
      Upload(std::span<const uint8_t>{ items.data(), items.size() * sizeof(T) }, context);
    }

  protected:
    winrt::com_ptr<ID3D11Buffer> _buffer;
    uint32_t _capacity;
    uint32_t _size;
  };
}