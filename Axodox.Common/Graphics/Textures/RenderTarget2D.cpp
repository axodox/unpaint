#include "pch.h"
#include "RenderTarget2D.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  RenderTarget2D::RenderTarget2D(const GraphicsDevice& device, const Texture2DDefinition& definition, DXGI_FORMAT format) :
    Texture2D(device, PrepareDefinition(definition))
  {
    if (format == DXGI_FORMAT_UNKNOWN)
    {
      format = definition.ViewFormat == DXGI_FORMAT_UNKNOWN ? definition.Format : definition.ViewFormat;
    }

    _renderTargetView = CreateView(format);
    _defaultViewport = GetDefaultViewport();
  }

  RenderTarget2D::RenderTarget2D(const GraphicsDevice& device, const winrt::com_ptr<ID3D11Texture2D>& texture, DXGI_FORMAT format) :
    Texture2D(device, texture)
  {
    _renderTargetView = CreateView(format == DXGI_FORMAT_UNKNOWN ? _definition.Format : format);
    _defaultViewport = GetDefaultViewport();
  }

  void RenderTarget2D::BindRenderTargetView(DXGI_FORMAT format, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    Unbind(context);
    
    context->BindRenderTargetView(GetView(format));
    (*context)->RSSetViewports(1, &_defaultViewport);
  }

  void RenderTarget2D::UnbindRenderTargetView(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();
    context->UnbindRenderTargetView(_renderTargetView.get());

    for (auto& [format, view] : _alternateRenderTargetViews)
    {
      context->UnbindRenderTargetView(view.get());
    }
  }

  void RenderTarget2D::Unbind(GraphicsDeviceContext* context)
  {
    Texture2D::Unbind(context);
    UnbindRenderTargetView(context);
  }

  void RenderTarget2D::Clear(const DirectX::XMFLOAT4& color, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->ClearRenderTargetView(_renderTargetView.get(), reinterpret_cast<const float*>(&color));
  }

  void RenderTarget2D::Discard(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->DiscardResource(_texture.get());
  }

  const D3D11_VIEWPORT& RenderTarget2D::Viewport() const
  {
    return _defaultViewport;
  }

  const winrt::com_ptr<ID3D11RenderTargetView>& RenderTarget2D::operator*() const
  {
    return _renderTargetView;
  }

  ID3D11RenderTargetView* RenderTarget2D::operator->() const
  {
    return _renderTargetView.get();
  }

  ID3D11RenderTargetView* RenderTarget2D::get() const
  {
    return _renderTargetView.get();
  }

  Texture2DDefinition RenderTarget2D::PrepareDefinition(Texture2DDefinition definition)
  {
    add_flag(definition.Flags, Texture2DFlags::RenderTarget);
    return definition;
  }

  winrt::com_ptr<ID3D11RenderTargetView> RenderTarget2D::CreateView(DXGI_FORMAT format) const
  {
    //Define dimension
    D3D11_RTV_DIMENSION dimension;
    if (_definition.TextureCount > 1u)
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
      }
      else
      {
        dimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
      }
    }
    else
    {
      if (_definition.SampleCount > 1u)
      {
        dimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
      }
      else
      {
        dimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      }
    }

    //Create view
    CD3D11_RENDER_TARGET_VIEW_DESC description(
      dimension,
      format,
      0u,
      0u,
      _definition.TextureCount
    );

    com_ptr<ID3D11RenderTargetView> result;
    check_hresult(_device->CreateRenderTargetView(_texture.get(), &description, result.put()));

    return result;
  }

  ID3D11RenderTargetView* RenderTarget2D::GetView(DXGI_FORMAT format)
  {
    ID3D11RenderTargetView* result;

    if (format != DXGI_FORMAT_UNKNOWN)
    {
      auto& alternateView = _alternateRenderTargetViews[format];
      if (!alternateView)
      {
        alternateView = CreateView(format);
      }

      result = alternateView.get();
    }
    else
    {
      result = _renderTargetView.get();
    }

    return result;
  }

  D3D11_VIEWPORT RenderTarget2D::GetDefaultViewport() const
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