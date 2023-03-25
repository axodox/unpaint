#pragma once
#include "pch.h"

namespace Axodox::Graphics
{
  typedef std::span<const D3D11_INPUT_ELEMENT_DESC> VertexDefition;

  struct VertexPosition
  {
    DirectX::XMFLOAT3 Position;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct VertexPositionColor
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMUBYTEN4 Color;
    
    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct VertexPositionTexture
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMUSHORTN2 Texture;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct VertexPositionNormal
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct VertexPositionNormalColor
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;
    DirectX::PackedVector::XMUBYTEN4 Color;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };

  struct VertexPositionNormalTexture
  {
    DirectX::XMFLOAT3 Position;
    DirectX::PackedVector::XMBYTEN4 Normal;
    DirectX::PackedVector::XMUSHORTN2 Texture;

    static const D3D11_INPUT_ELEMENT_DESC Definition[];
  };
}