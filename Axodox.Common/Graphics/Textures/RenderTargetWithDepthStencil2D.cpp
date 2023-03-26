#include "pch.h"
#include "RenderTargetWithDepthStencil2D.h"

using namespace std;

namespace Axodox::Graphics
{
  RenderTargetWithDepthStencil2D::RenderTargetWithDepthStencil2D(const GraphicsDevice& device, const Texture2DDefinition& definition, DXGI_FORMAT depthFormat) :
    GraphicsResource(device),
    _renderTarget(device, definition),
    _depthStencil(device, PrepareDepthStencilDefinition(definition, depthFormat))
  { }

  RenderTarget2D* RenderTargetWithDepthStencil2D::RenderTarget()
  {
    return &_renderTarget;
  }

  DepthStencil2D* RenderTargetWithDepthStencil2D::DepthStencil()
  {
    return &_depthStencil;
  }

  void RenderTargetWithDepthStencil2D::SetAsRenderTarget(GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    context->BindRenderTargets({ &_renderTarget, 1 }, &_depthStencil);
  }

  void RenderTargetWithDepthStencil2D::Clear(const DirectX::XMFLOAT4& color, float depth, GraphicsDeviceContext* context)
  {
    _renderTarget.Clear(color, context);
    _depthStencil.Clear(depth, context);
  }

  void RenderTargetWithDepthStencil2D::Discard(GraphicsDeviceContext* context)
  {
    _renderTarget.Discard(context);
    _depthStencil.Discard(context);
  }

  Texture2DDefinition RenderTargetWithDepthStencil2D::PrepareDepthStencilDefinition(Texture2DDefinition definition, DXGI_FORMAT depthFormat)
  {
    definition.Format = depthFormat;
    definition.ViewFormat = DXGI_FORMAT_UNKNOWN;

    return definition;
  }
}