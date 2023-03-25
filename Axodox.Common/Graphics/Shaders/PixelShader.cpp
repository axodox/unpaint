#include "pch.h"
#include "PixelShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  PixelShader::PixelShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device)
  {
    check_hresult(_device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void PixelShader::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->PSSetShader(_shader.get(), nullptr, 0);
  }
}