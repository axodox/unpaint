#include "pch.h"
#include "VertexShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  VertexShader::VertexShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device),
    _source(buffer.begin(), buffer.end())
  {
    check_hresult(_device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void VertexShader::InputLayout(VertexDefition vertexDefinition)
  {
    if (_layout.first == vertexDefinition.data()) return;

    auto& layout = _layouts[vertexDefinition.data()];
    if (!layout)
    {
      check_hresult(_device->CreateInputLayout(
        vertexDefinition.data(),
        uint32_t(vertexDefinition.size()),
        _source.data(),
        _source.size(),
        layout.put()));
    }

    _layout = { vertexDefinition.data(), layout.get() };
  }

  void VertexShader::Bind(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->VSSetShader(_shader.get(), nullptr, 0);
    (*context)->IASetInputLayout(_layout.second);
  }
}