#include "pch.h"
#include "HullShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  HullShader::HullShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device)
  {
    check_hresult(_device->CreateHullShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void HullShader::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->HSSetShader(_shader.get(), nullptr, 0);
  }
}