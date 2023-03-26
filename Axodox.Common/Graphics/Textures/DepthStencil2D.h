#pragma once
#include "Texture2D.h"

namespace Axodox::Graphics
{
  class DepthStencil2D : public Texture2D
  {
  public:
    DepthStencil2D(const GraphicsDevice& device, const Texture2DDefinition& definition);

    void BindDepthStencilView(GraphicsDeviceContext* context = nullptr);
    void UnbindDepthStencilView(GraphicsDeviceContext* context = nullptr);

    virtual void Unbind(GraphicsDeviceContext* context = nullptr) override;

    void Clear(float depth = 1.f, GraphicsDeviceContext* context = nullptr);
    void Discard(GraphicsDeviceContext* context = nullptr);

    const winrt::com_ptr<ID3D11DepthStencilView>& operator*() const;
    ID3D11DepthStencilView* operator->() const;
    ID3D11DepthStencilView* get() const;

  private:
    winrt::com_ptr<ID3D11DepthStencilView> _depthStencilView;
    D3D11_VIEWPORT _defaultViewport;

    static Texture2DDefinition PrepareDefinition(Texture2DDefinition definition);
    winrt::com_ptr<ID3D11DepthStencilView> CreateView() const;
    D3D11_VIEWPORT GetDefaultViewport() const;
  };
}