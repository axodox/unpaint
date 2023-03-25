#pragma once
#include "GraphicsTypes.h"

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

  class GraphicsDeviceContext
  {
  public:
    GraphicsDeviceContext() = default;
    explicit GraphicsDeviceContext(const winrt::com_ptr<ID3D11DeviceContextT>& context);

    const winrt::com_ptr<ID3D11DeviceContextT>& operator*() const;
    ID3D11DeviceContextT* operator->() const;
    ID3D11DeviceContextT* get() const;

    void BindShaderResourceView(ID3D11ShaderResourceView* view, ShaderStage stage, uint32_t slot);

    void BindShaders(VertexShader* vertexShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, GeometryShader* geometryShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, PixelShader* pixelShader);
    void BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, GeometryShader* geometryShader, PixelShader* pixelShader);

  private:
    winrt::com_ptr<ID3D11DeviceContextT> _context;
  };
}