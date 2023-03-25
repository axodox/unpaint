#include "pch.h"
#include "DomainShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  DomainShader::DomainShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device)
  {
    check_hresult(_device->CreateDomainShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void DomainShader::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->DSSetShader(_shader.get(), nullptr, 0);
  }
}