#include "pch.h"
#include "StagingTexture2D.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Collections/AlignedAllocator.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Collections;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  StagingTexture2D::StagingTexture2D(const GraphicsDevice& device, const Texture2DDefinition& definition) :
    Texture2D(device, PrepareDefinition(definition))
  { }

  TextureData StagingTexture2D::Download(uint32_t textureIndex, GraphicsDeviceContext * context)
  {
    if (!context) context = _device.ImmediateContext();

    //Map texture
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    zero_memory(mappedSubresource);

    auto subresourceIndex = D3D11CalcSubresource(0, textureIndex, _definition.MipCount);
    check_hresult((*context)->Map(_texture.get(), subresourceIndex, D3D11_MAP_READ, 0, &mappedSubresource));

    //Copy data
    aligned_vector<uint8_t> buffer;
    buffer.resize(_definition.Height * mappedSubresource.RowPitch);    
    memcpy(buffer.data(), mappedSubresource.pData, buffer.size());

    //Unmap texture
    (*context)->Unmap(_texture.get(), 0);

    //Return result
    TextureData result{
      _definition.Width,
      _definition.Height,
      mappedSubresource.RowPitch,
      _definition.Format,
      move(buffer)
    };
    
    return result;
  }
    
  Texture2DDefinition StagingTexture2D::PrepareDefinition(Texture2DDefinition definition)
  {
    definition.SampleCount = 1;
    definition.SampleQuality = 0;
    definition.MipCount = 1;
    definition.Flags = Texture2DFlags::Staging;
    return definition;
  }
}