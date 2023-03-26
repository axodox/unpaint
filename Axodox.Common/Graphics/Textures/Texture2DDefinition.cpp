#include "pch.h"
#include "Texture2DDefinition.h"
#include "Infrastructure/BitwiseOperations.h"

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

    //Sampling
    result.SampleDesc.Count = SampleCount;
    result.SampleDesc.Quality = SampleQuality;

    //Usage flags
    if (has_flag(Flags, Texture2DFlags::Updateable))
    {
      result.Usage = D3D11_USAGE_DYNAMIC;
      result.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }

    if (has_flag(Flags, Texture2DFlags::Staging))
    {
      result.Usage = D3D11_USAGE_STAGING;
      result.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    }

    //Bind flags
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

    //Misc flags
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
    Texture2DDefinition result{
      description.Width,
      description.Height,
      description.Format,
      description.SampleDesc.Count,
      description.SampleDesc.Quality,
      description.MipLevels,
      description.ArraySize
    };

    //Usage flags
    add_flag(result.Flags, Texture2DFlags::Updateable, description.Usage == D3D11_USAGE_DYNAMIC);
    add_flag(result.Flags, Texture2DFlags::Staging, description.Usage == D3D11_USAGE_STAGING);

    //Bind flags
    add_flag(result.Flags, Texture2DFlags::RenderTarget, description.BindFlags & D3D11_BIND_RENDER_TARGET);
    add_flag(result.Flags, Texture2DFlags::DepthStencil, description.BindFlags & D3D11_BIND_DEPTH_STENCIL);
    add_flag(result.Flags, Texture2DFlags::Unordered, description.BindFlags & D3D11_BIND_UNORDERED_ACCESS);

    //Misc flags
    add_flag(result.Flags, Texture2DFlags::GenerateMips, description.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS);
    add_flag(result.Flags, Texture2DFlags::Cube, description.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);

    return result;
  }
}