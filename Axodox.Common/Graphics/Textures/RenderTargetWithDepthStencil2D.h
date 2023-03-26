#pragma once
#include "RenderTarget2D.h"
#include "DepthStencil2D.h"

namespace Axodox::Graphics
{
  class RenderTargetWithDepthStencil2D : public GraphicsResource
  {
  public:
    RenderTargetWithDepthStencil2D(const GraphicsDevice& device, const Texture2DDefinition& definition, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
    
    RenderTarget2D* RenderTarget();
    DepthStencil2D* DepthStencil();

    void SetAsRenderTarget(GraphicsDeviceContext* context = nullptr);
    void Clear(const DirectX::XMFLOAT4& color = { 0.f, 0.f, 0.f, 0.f }, float depth = 1.f, GraphicsDeviceContext* context = nullptr);
    void Discard(GraphicsDeviceContext* context = nullptr);

  private:
    RenderTarget2D _renderTarget;
    DepthStencil2D _depthStencil;

    static Texture2DDefinition PrepareDepthStencilDefinition(Texture2DDefinition definition, DXGI_FORMAT depthFormat);
  };
}