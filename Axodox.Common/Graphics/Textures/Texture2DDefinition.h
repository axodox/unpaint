#pragma once
#include "pch.h"

namespace Axodox::Graphics
{
  enum class Texture2DFlags : uint32_t
  {
    None = 0,
    Updateable = 1,
    Unordered = 2,
    RenderTarget = 4,
    DepthStencil = 8,
    Array = 16,
    Cube = 32,
    GenerateMips = 64,
    Staging = 128
  };

  struct Texture2DDefinition
  {
    uint32_t Width = 0;
    uint32_t Height = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    uint32_t SampleCount = 1;
    uint32_t SampleQuality = 0;
    uint32_t MipCount = 1;
    uint32_t TextureCount = 1;
    Texture2DFlags Flags = Texture2DFlags::None;
    DXGI_FORMAT ViewFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT UnorderedFormat = DXGI_FORMAT_UNKNOWN;

    Texture2DDefinition() = default;

    Texture2DDefinition(
      uint32_t width,
      uint32_t height,
      DXGI_FORMAT format,
      Texture2DFlags flags,
      uint32_t mipCount = 1,
      DXGI_FORMAT viewFormat = DXGI_FORMAT_UNKNOWN);

    Texture2DDefinition(
      uint32_t width,
      uint32_t height,
      DXGI_FORMAT format,
      uint32_t sampleCount = 1,
      uint32_t sampleQuality = 0,
      uint32_t mipCount = 1,
      uint32_t textureCount = 1,
      Texture2DFlags flags = Texture2DFlags::None);

    D3D11_TEXTURE2D_DESC ToDescription() const;

    static Texture2DDefinition FromDescription(const D3D11_TEXTURE2D_DESC& description);
  };
}