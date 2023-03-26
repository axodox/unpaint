#pragma once
#include "GraphicsTypes.h"
#include "Collections/Hasher.h"

namespace Axodox::Graphics
{
  enum class ShaderStage
  {
    Vertex,
    Hull,
    Domain,
    Geometry,
    Pixel,
    Compute
  };

  class VertexShader;
  class HullShader;
  class DomainShader;
  class GeometryShader;
  class PixelShader;

  class RenderTarget2D;
  class DepthStencil2D;

  class GraphicsDeviceContext
  {
  public:
    GraphicsDeviceContext() = default;
    explicit GraphicsDeviceContext(const winrt::com_ptr<ID3D11DeviceContextT>& context);

    const winrt::com_ptr<ID3D11DeviceContextT>& operator*() const;
    ID3D11DeviceContextT* operator->() const;
    ID3D11DeviceContextT* get() const;

    void BindShaderResourceView(ID3D11ShaderResourceView* view, ShaderStage stage, uint32_t slot);
    void UnbindShaderResourceView(ID3D11ShaderResourceView* view);

    void BindUnorderedAccessView(ID3D11UnorderedAccessView* view, uint32_t slot);
    void UnbindUnorderedAccessView(ID3D11UnorderedAccessView* view);

    void BindRenderTargetView(ID3D11RenderTargetView* view);
    void UnbindRenderTargetView(ID3D11RenderTargetView* view);

    void BindDepthStencilView(ID3D11DepthStencilView* view);
    void UnbindDepthStencilView(ID3D11DepthStencilView* view);

    void BindRenderTargets(std::span<RenderTarget2D> renderTargets, DepthStencil2D* depthStencil);

    void BindShaders(VertexShader* vertexShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, GeometryShader* geometryShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, GeometryShader* geometryShader, PixelShader* pixelShader);

  private:
    winrt::com_ptr<ID3D11DeviceContextT> _context;
    std::map<ShaderStage, std::map<uint32_t, ID3D11ShaderResourceView*>> _boundShaderResourceViews;
    std::map<uint32_t, ID3D11UnorderedAccessView*> _boundUnorderedAccessViews;
    std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> _boundRenderTargetViews;
    ID3D11DepthStencilView* _boundDepthStencilView = nullptr;
  };
}