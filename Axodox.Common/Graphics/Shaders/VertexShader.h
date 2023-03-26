#pragma once
#include "Graphics\Devices\GraphicsResource.h"
#include "Graphics\Meshes\VertexDefinitions.h"

namespace Axodox::Graphics
{
  class VertexShader : public GraphicsResource
  {
  public:
    VertexShader(const GraphicsDevice& device, std::span<const uint8_t> buffer);

    void InputLayout(VertexDefition layout);
    void Bind(GraphicsDeviceContext* context = nullptr);

  private:
    std::vector<uint8_t> _source;
    winrt::com_ptr<ID3D11VertexShader> _shader;

    std::map<const D3D11_INPUT_ELEMENT_DESC*, winrt::com_ptr<ID3D11InputLayout>> _layouts;
    std::pair<const D3D11_INPUT_ELEMENT_DESC*, ID3D11InputLayout*> _layout = {};
  };
}