#include "pch.h"
#include "ConstantBuffer.h"

using namespace std;

namespace Axodox::Graphics
{
  ConstantBuffer::ConstantBuffer(const GraphicsDevice& device, uint32_t size) :
    GraphicsBuffer(device, size, BufferType::Constant)
  { }

  void ConstantBuffer::Bind(ShaderStage stage, uint32_t slot, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    auto buffer = _buffer.get();
    switch (stage)
    {
    case ShaderStage::Vertex:
      (*context)->VSSetConstantBuffers(slot, 1, &buffer);
      break;
    case ShaderStage::Hull:
      (*context)->HSSetConstantBuffers(slot, 1, &buffer);
      break;
    case ShaderStage::Domain:
      (*context)->DSSetConstantBuffers(slot, 1, &buffer);
      break;
    case ShaderStage::Geometry:
      (*context)->GSSetConstantBuffers(slot, 1, &buffer);
      break;
    case ShaderStage::Pixel:
      (*context)->PSSetConstantBuffers(slot, 1, &buffer);
      break;
    case ShaderStage::Compute:
      (*context)->CSSetConstantBuffers(slot, 1, &buffer);
      break;
    default:
      throw logic_error("Shader stage not implemented");
    }
  }
}