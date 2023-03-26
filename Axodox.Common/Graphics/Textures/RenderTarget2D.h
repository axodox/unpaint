#pragma once
#include "Texture2D.h"

namespace Axodox::Graphics
{
  class RenderTarget2D : public Texture2D
  {
  public:
    RenderTarget2D(const GraphicsDevice& device, const Texture2DDefinition& definition, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);
    RenderTarget2D(const GraphicsDevice& device, const winrt::com_ptr<ID3D11Texture2D>& texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

    void BindRenderTargetView(DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, GraphicsDeviceContext* context = nullptr);
    void UnbindRenderTargetView(GraphicsDeviceContext* context = nullptr);

    virtual void Unbind(GraphicsDeviceContext* context = nullptr) override;

    void Clear(const DirectX::XMFLOAT4& color = { 0.f, 0.f, 0.f, 0.f }, GraphicsDeviceContext* context = nullptr);
    void Discard(GraphicsDeviceContext* context = nullptr);

    const D3D11_VIEWPORT& Viewport() const;

    const winrt::com_ptr<ID3D11RenderTargetView>& operator*() const;
    ID3D11RenderTargetView* operator->() const;
    ID3D11RenderTargetView* get() const;

  private:
    winrt::com_ptr<ID3D11RenderTargetView> _renderTargetView;
    std::map<DXGI_FORMAT, winrt::com_ptr<ID3D11RenderTargetView>> _alternateRenderTargetViews;
    D3D11_VIEWPORT _defaultViewport;

    static Texture2DDefinition PrepareDefinition(Texture2DDefinition definition);
    winrt::com_ptr<ID3D11RenderTargetView> CreateView(DXGI_FORMAT format) const;
    ID3D11RenderTargetView* GetView(DXGI_FORMAT format);
    D3D11_VIEWPORT GetDefaultViewport() const;
  };
}