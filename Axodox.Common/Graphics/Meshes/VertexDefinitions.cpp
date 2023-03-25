#include "pch.h"
#include "VertexDefinitions.h"

namespace Axodox::Graphics
{
  consteval D3D11_INPUT_ELEMENT_DESC InputElement(const char* semanticName, uint32_t semanticIndex, DXGI_FORMAT format)
  {
    return {
      .SemanticName = semanticName,
      .SemanticIndex = semanticIndex,
      .Format = format,
      .InputSlot = 0,
      .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    };
  }

  const D3D11_INPUT_ELEMENT_DESC VertexPosition::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT)
  };

  const D3D11_INPUT_ELEMENT_DESC VertexPositionColor::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT),
    InputElement("COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM)
  };

  const D3D11_INPUT_ELEMENT_DESC VertexPositionTexture::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT),
    InputElement("TEXCOORD", 0, DXGI_FORMAT_R16G16_UNORM)
  };

  const D3D11_INPUT_ELEMENT_DESC VertexPositionNormal::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT),
    InputElement("NORMAL", 0, DXGI_FORMAT_R8G8B8A8_SNORM)
  };
  
  const D3D11_INPUT_ELEMENT_DESC VertexPositionNormalColor::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT),
    InputElement("NORMAL", 0, DXGI_FORMAT_R8G8B8A8_SNORM),
    InputElement("COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM)
  };

  const D3D11_INPUT_ELEMENT_DESC VertexPositionNormalTexture::Definition[] = {
    InputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT),
    InputElement("NORMAL", 0, DXGI_FORMAT_R8G8B8A8_SNORM),
    InputElement("TEXCOORD", 0, DXGI_FORMAT_R16G16_UNORM)
  };
}