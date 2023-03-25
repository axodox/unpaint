#include "pch.h"
#include "GeometryShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  GeometryShader::GeometryShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device)
  {
    check_hresult(_device->CreateGeometryShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void GeometryShader::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->GSSetShader(_shader.get(), nullptr, 0);
  }
}