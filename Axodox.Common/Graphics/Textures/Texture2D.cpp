#include "pch.h"
#include "Texture2D.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  Texture2D::Texture2D(const GraphicsDevice& device, const Texture2DDefinition& definition) :
    GraphicsResource(device),
    _definition(definition)
  {
    auto description = definition.ToDescription();
    check_hresult(_device->CreateTexture2D(&description, nullptr, _texture.put()));

    //No views for staging textures
    if (has_flag(definition.Flags, Texture2DFlags::Staging)) return;

    //Create shader resource views
    auto viewFormat = definition.ViewFormat == DXGI_FORMAT_UNKNOWN ? _definition.Format : definition.ViewFormat;
    auto isArray = has_flag(definition.Flags, Texture2DFlags::Array) || _definition.TextureCount > 1u;
    auto isCube = has_flag(definition.Flags, Texture2DFlags::Cube);
    InitializeShaderResourceViews(viewFormat, isCube, isArray);

    //Create unordered access views
    if (has_flag(definition.Flags, Texture2DFlags::Unordered))
    {
      auto uavFormat = definition.UnorderedFormat == DXGI_FORMAT_UNKNOWN ? viewFormat : definition.UnorderedFormat;
      InitializeUnorderedViews(uavFormat, isArray);
    }
  }

  Texture2D::Texture2D(const GraphicsDevice& device, const TextureData& texture) :
    GraphicsResource(device)
  {
    _definition.Width = texture.Width;
    _definition.Height = texture.Height;
    _definition.Format = texture.Format;

    CD3D11_TEXTURE2D_DESC description(
      _definition.Format,
      _definition.Width,
      _definition.Format,
      _definition.TextureCount,
      _definition.MipCount,
      D3D11_BIND_SHADER_RESOURCE,
      D3D11_USAGE_IMMUTABLE
    );

    D3D11_SUBRESOURCE_DATA subresourceData{
      texture.Buffer.data(), texture.Stride, 0u
    };

    check_hresult(_device->CreateTexture2D(&description, &subresourceData, _texture.put()));

    InitializeShaderResourceViews();
  }

  Texture2D::Texture2D(const GraphicsDevice& device, const winrt::com_ptr<ID3D11Texture2D>& texture) :
    GraphicsResource(device),
    _texture(texture)
  {
    D3D11_TEXTURE2D_DESC description;
    _texture->GetDesc(&description);

    _definition = Texture2DDefinition::FromDescription(description);

    if (description.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
      InitializeShaderResourceViews();
    }
  }

  const Texture2DDefinition& Texture2D::Definition() const
  {
    return _definition;
  }

  void Texture2D::BindShaderResourceView(ShaderStage stage, uint32_t slot, DXGI_FORMAT format, uint32_t mip, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    UnbindUnorderedAccessView(mip, context);

    auto views = GetResourceViews({ format, mip });
    for (auto i = 0u; i < views.size(); i++)
    {
      context->BindShaderResourceView(views[i], stage, slot + i);
    }
  }

  void Texture2D::UnbindShaderResourceView(uint32_t mip, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    for (auto& view : _resourceViews)
    {
      context->UnbindShaderResourceView(view.get());
    }

    for (auto& [key, view] : _alternateResourceViews)
    {
      if (key.Mip != ~0u && mip != ~0u && key.Mip != mip) continue;
      context->UnbindShaderResourceView(view.get());
    }
  }

  void Texture2D::BindUnorderedAccessView(uint32_t slot, DXGI_FORMAT format, uint32_t mip, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    UnbindShaderResourceView(mip, context);
   
    auto view = GetUnorderedView({ format, mip });
    context->BindUnorderedAccessView(view, slot);
  }

  void Texture2D::UnbindUnorderedAccessView(uint32_t mip, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    context->UnbindUnorderedAccessView(_unorderedView.get());

    for (auto& [key, view] : _alternateUnorderedViews)
    {
      if (key.Mip != ~0u && mip != ~0u && key.Mip != mip) continue;
      context->UnbindUnorderedAccessView(view.get());
    }
  }

  void Texture2D::Unbind(GraphicsDeviceContext* context)
  {
    UnbindShaderResourceView(~0u, context);
    UnbindUnorderedAccessView(~0u, context);
  }

  void Texture2D::Update(const TextureData& texture, uint32_t subresourceIndex, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    Unbind(context);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    check_hresult((*context)->Map(_texture.get(), subresourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));

    auto height = min(texture.ByteCount() / texture.Stride, texture.Height);
    auto stride = min(texture.Stride, mappedSubresource.RowPitch);
    for (auto row = 0u; row < height; row++)
    {
      memcpy(
        (uint8_t*)mappedSubresource.pData + row * mappedSubresource.RowPitch,
        texture.Buffer.data() + texture.Stride * row,
        stride);
    }

    (*context)->Unmap(_texture.get(), 0);
  }

  void Texture2D::GenerateMips(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    if (has_flag(_definition.Flags, Texture2DFlags::GenerateMips))
    {
      (*context)->GenerateMips(_resourceViews.front().get());
    }
    else
    {
      throw logic_error("Mipmap generation is not supported on this texture.");
    }
  }

  void Texture2D::Copy(Texture2D* target, uint32_t selectedMip, GraphicsDeviceContext* context)
  {
    if (this == target) return;
    if (!context) context = _device.ImmediateContext();

    Unbind(context);
    target->Unbind(context);

    auto textureCount = min(_definition.TextureCount, target->_definition.TextureCount);
    auto mipCount = min(_definition.MipCount, target->_definition.MipCount);
    for (auto textureIndex = 0u; textureIndex < textureCount; textureIndex++)
    {
      for (auto mipIndex = 0u; mipIndex < mipCount; mipIndex++)
      {
        if (selectedMip != ~0u && mipIndex != selectedMip) continue;

        auto destionationIndex = D3D11CalcSubresource(mipIndex, textureIndex, target->_definition.MipCount);
        auto sourceIndex = D3D11CalcSubresource(mipIndex, textureIndex, _definition.MipCount);

        if (_definition.SampleCount != target->_definition.SampleCount)
        {
          (*context)->ResolveSubresource(target->_texture.get(), destionationIndex, _texture.get(), sourceIndex, target->_definition.Format);
        }
        else
        {
          (*context)->CopySubresourceRegion(
            target->_texture.get(),
            destionationIndex,
            0u,
            0u,
            0u,
            _texture.get(),
            sourceIndex,
            nullptr);
        }        
      }
    }
  }

  const winrt::com_ptr<ID3D11Texture2D>& Texture2D::operator*() const
  {
    return _texture;
  }

  ID3D11Texture2D* Texture2D::operator->() const
  {
    return _texture.get();
  }

  ID3D11Texture2D* Texture2D::get() const
  {
    return _texture.get();
  }

  void Texture2D::InitializeShaderResourceViews(DXGI_FORMAT format, bool isCube, bool isArray)
  {
    if (format == DXGI_FORMAT_UNKNOWN) format = _definition.Format;

    //Define view formats
    vector<DXGI_FORMAT> viewFormats;
    switch (format)
    {
    case DXGI_FORMAT_YUY2:
      //viewFormats.push_back(DXGI_FORMAT_R8G8_B8G8_UNORM);
      viewFormats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
      break;
    case DXGI_FORMAT_NV12:
      viewFormats.push_back(DXGI_FORMAT_R8_UNORM);
      viewFormats.push_back(DXGI_FORMAT_R8G8_UNORM);
      break;
    default:
      viewFormats.push_back(format);
      break;
    }

    //Select dimension
    D3D11_SRV_DIMENSION dimension;
    if (_definition.TextureCount > 1u || isArray)
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
      }
      else
      {
        if (isCube)
        {
          if (_definition.TextureCount == 6u)
          {
            dimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
          }
          else
          {
            dimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
          }
        }
        else
        {
          dimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        }
      }
    }
    else
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
      }
      else
      {
        dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      }
    }

    //Create resource views
    _resourceViews.resize(viewFormats.size());
    for (auto textureIndex = 0u; textureIndex < viewFormats.size(); textureIndex++)
    {
      CD3D11_SHADER_RESOURCE_VIEW_DESC description(
        dimension,
        viewFormats[textureIndex],
        0u,
        _definition.MipCount,
        0u,
        _definition.TextureCount
      );

      check_hresult(_device->CreateShaderResourceView(_texture.get(), &description, _resourceViews[textureIndex].put()));
    }
  }

  std::vector<ID3D11ShaderResourceView*> Axodox::Graphics::Texture2D::GetResourceViews(ResourceViewKey key)
  {
    if (has_flag(_definition.Flags, Texture2DFlags::Staging)) throw logic_error("Staging textures cannot be bound.");

    vector<ID3D11ShaderResourceView*> _views;
    if (is_default(key))
    {
      if (_resourceViews.empty())
      {
        throw logic_error("This texture does not have a default shader resource view.");
      }

      _views.reserve(_resourceViews.size());
      for (auto& view : _resourceViews)
      {
        _views.push_back(view.get());
      }
    }
    else
    {
      if (key.Format == DXGI_FORMAT_UNKNOWN) key.Format = _definition.Format;

      auto& view = _alternateResourceViews[key];
      if (view == nullptr)
      {
        CD3D11_SHADER_RESOURCE_VIEW_DESC description(
          has_flag(_definition.Flags, Texture2DFlags::Array) || _definition.TextureCount > 1u ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D,
          key.Format,
          key.Mip == uint32_t(-1) ? 0u : key.Mip,
          key.Mip == uint32_t(-1) ? _definition.MipCount : 1u,
          0u,
          _definition.TextureCount
        );

        check_hresult(_device->CreateShaderResourceView(_texture.get(), &description, view.put()));
      }
      _views.push_back(view.get());
    }

    return _views;
  }
  
  void Texture2D::InitializeUnorderedViews(DXGI_FORMAT format, bool isArray)
  {
    D3D11_UNORDERED_ACCESS_VIEW_DESC description{};
    description.Format = format;
    description.ViewDimension = isArray ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;
    description.Texture2DArray.ArraySize = _definition.TextureCount;

    check_hresult(_device->CreateUnorderedAccessView(_texture.get(), &description, _unorderedView.put()));
  }

  ID3D11UnorderedAccessView* Texture2D::GetUnorderedView(UnorderedViewKey key)
  {
    if (has_flag(_definition.Flags, Texture2DFlags::Staging)) throw logic_error("Staging textures cannot be bound.");

    if (is_default(key))
    {
      if (!_unorderedView)
      {
        throw logic_error("This texture does not support unordered access.");
      }

      return _unorderedView.get();
    }
    else
    {
      if (key.Format == DXGI_FORMAT_UNKNOWN) key.Format = _definition.Format;
      if (key.Mip == uint32_t(-1)) key.Mip = 0u;

      auto& view = _alternateUnorderedViews[key];
      if (!view)
      {
        D3D11_UNORDERED_ACCESS_VIEW_DESC description{};
        description.Format = key.Format;
        description.ViewDimension = has_flag(_definition.Flags, Texture2DFlags::Array) || _definition.TextureCount > 1u ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;
        description.Texture2DArray.MipSlice = key.Mip;
        description.Texture2DArray.ArraySize = _definition.TextureCount;

        check_hresult(_device->CreateUnorderedAccessView(_texture.get(), &description, view.put()));
      }
      return view.get();
    }
  }
}