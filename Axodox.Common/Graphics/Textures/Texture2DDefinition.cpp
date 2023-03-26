#include "pch.h"
#include "Texture2DDefinition.h"
#include "Infrastructure\BitwiseOperations.h"

using namespace Axodox::Infrastructure;

namespace Axodox::Graphics
{
  Texture2DDefinition::Texture2DDefinition(
    uint32_t width,
    uint32_t height,
    DXGI_FORMAT format,
    Texture2DFlags flags,
    uint32_t mipCount,
    DXGI_FORMAT viewFormat) :
    Width(width),
    Height(height),
    Format(format),
    Flags(flags),
    MipCount(mipCount),
    ViewFormat(viewFormat)
  { }

  Texture2DDefinition::Texture2DDefinition(
    uint32_t width,
    uint32_t height,
    DXGI_FORMAT format,
    uint32_t sampleCount,
    uint32_t sampleQuality,
    uint32_t mipCount,
    uint32_t textureCount,
    Texture2DFlags flags) :
    Width(width),
    Height(height),
    Format(format),
    SampleCount(sampleCount),
    SampleQuality(sampleQuality),
    MipCount(mipCount),
    TextureCount(textureCount),
    Flags(flags)
  { }
  
  D3D11_TEXTURE2D_DESC Texture2DDefinition::ToDescription() const
  {
    CD3D11_TEXTURE2D_DESC result(
      Format,
      Width,
      Height,
      TextureCount,
      MipCount,
      D3D11_BIND_SHADER_RESOURCE
    );

    result.SampleDesc.Count = SampleCount;
    result.SampleDesc.Quality = SampleQuality;

    if (has_flag(Flags, Texture2DFlags::Updateable))
    {
      result.Usage = D3D11_USAGE_DYNAMIC;
      result.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }

    if (has_flag(Flags, Texture2DFlags::RenderTarget))
    {
      result.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    if (has_flag(Flags, Texture2DFlags::DepthStencil))
    {
      result.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }

    if (has_flag(Flags, Texture2DFlags::Unordered))
    {
      result.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    if (has_flag(Flags, Texture2DFlags::GenerateMips))
    {
      result.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    if (has_flag(Flags, Texture2DFlags::Cube))
    {
      result.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    return result;
  }
  
  Texture2DDefinition Texture2DDefinition::FromDescription(const D3D11_TEXTURE2D_DESC& description)
  {
    return Texture2DDefinition(
      description.Width,
      description.Height,
      description.Format,
      description.SampleDesc.Count,
      description.SampleDesc.Quality,
      description.MipLevels,
      description.ArraySize
    );
  }
}