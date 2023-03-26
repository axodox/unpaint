#include "pch.h"
#include "DepthStencil2D.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  DepthStencil2D::DepthStencil2D(const GraphicsDevice& device, const Texture2DDefinition& definition) :
    Texture2D(device, PrepareDefinition(definition))
  {
    _depthStencilView = CreateView();
    _defaultViewport = GetDefaultViewport();
  }

  void DepthStencil2D::BindDepthStencilView(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    Unbind(context);

    context->BindDepthStencilView(_depthStencilView.get());
    (*context)->RSSetViewports(1, &_defaultViewport);
  }

  void DepthStencil2D::UnbindDepthStencilView(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    context->UnbindDepthStencilView(_depthStencilView.get());
  }

  void DepthStencil2D::Unbind(GraphicsDeviceContext* context)
  {
    Texture2D::Unbind(context);
    UnbindDepthStencilView(context);
  }

  void DepthStencil2D::Clear(float depth, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->ClearDepthStencilView(_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0);
  }

  void DepthStencil2D::Discard(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->DiscardResource(_texture.get());
  }

  const winrt::com_ptr<ID3D11DepthStencilView>& DepthStencil2D::operator*() const
  {
    return _depthStencilView;
  }

  ID3D11DepthStencilView* DepthStencil2D::operator->() const
  {
    return _depthStencilView.get();
  }

  ID3D11DepthStencilView* DepthStencil2D::get() const
  {
    return _depthStencilView.get();
  }

  Texture2DDefinition DepthStencil2D::PrepareDefinition(Texture2DDefinition definition)
  {
    add_flag(definition.Flags, Texture2DFlags::DepthStencil);

    if (definition.ViewFormat == DXGI_FORMAT_UNKNOWN)
    {
      switch (definition.Format)
      {
      case DXGI_FORMAT_D16_UNORM:
        definition.ViewFormat = DXGI_FORMAT_R16_UNORM;
        definition.Format = DXGI_FORMAT_R16_TYPELESS;
        break;
      case DXGI_FORMAT_D32_FLOAT:
        definition.ViewFormat = DXGI_FORMAT_R32_FLOAT;
        definition.Format = DXGI_FORMAT_R32_TYPELESS;
        break;
      }
    }

    return definition;
  }

  winrt::com_ptr<ID3D11DepthStencilView> DepthStencil2D::CreateView() const
  {
    //Define dimension
    D3D11_DSV_DIMENSION dimension;
    if (_definition.TextureCount > 1u)
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
      }
      else
      {
        dimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
      }
    }
    else
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
      }
      else
      {
        dimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      }
    }

    //Definie view format
    auto viewFormat = _definition.ViewFormat == DXGI_FORMAT_UNKNOWN ? _definition.Format : _definition.ViewFormat;

    //Create view
    CD3D11_DEPTH_STENCIL_VIEW_DESC description(
      dimension,
      viewFormat,
      0u,
      0u,
      _definition.TextureCount
    );

    com_ptr<ID3D11DepthStencilView> result;
    check_hresult(_device->CreateDepthStencilView(_texture.get(), &description, result.put()));

    return result;
  }

  D3D11_VIEWPORT DepthStencil2D::GetDefaultViewport() const
  {
    return CD3D11_VIEWPORT(
      0.f,
      0.f,
      float(_definition.Width),
      float(_definition.Height),
      D3D11_MIN_DEPTH,
      D3D11_MAX_DEPTH
    );;
  }
}