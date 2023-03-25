#include "pch.h"
#include "GraphicsDeviceContext.h"

#include "..\Shaders\VertexShader.h"
#include "..\Shaders\HullShader.h"
#include "..\Shaders\DomainShader.h"
#include "..\Shaders\GeometryShader.h"
#include "..\Shaders\PixelShader.h"

using namespace std;

namespace Axodox::Graphics
{
  GraphicsDeviceContext::GraphicsDeviceContext(const winrt::com_ptr<ID3D11DeviceContextT>& context) :
    _context(context)
  { }

  const winrt::com_ptr<ID3D11DeviceContextT>& GraphicsDeviceContext::operator*() const
  {
    return _context;
  }

  ID3D11DeviceContextT* GraphicsDeviceContext::operator->() const
  {
    return _context.get();
  }

  ID3D11DeviceContextT* GraphicsDeviceContext::get() const
  {
    return _context.get();
  }
  
  void GraphicsDeviceContext::BindShaderResourceView(ID3D11ShaderResourceView* view, ShaderStage stage, uint32_t slot)
  {
    switch (stage)
    {
    case ShaderStage::Vertex:
      _context->VSSetShaderResources(slot, 1, &view);
      break;
    case ShaderStage::Hull:
      _context->HSSetShaderResources(slot, 1, &view);
      break;
    case ShaderStage::Domain:
      _context->DSSetShaderResources(slot, 1, &view);
      break;
    case ShaderStage::Geometry:
      _context->GSSetShaderResources(slot, 1, &view);
      break;
    case ShaderStage::Pixel:
      _context->PSSetShaderResources(slot, 1, &view);
      break;
    case ShaderStage::Compute:
      _context->CSSetShaderResources(slot, 1, &view);
      break;
    default:
      throw logic_error("Shader stage not implemented!");
    }
  }

  void GraphicsDeviceContext::BindShaders(VertexShader* vertexShader, PixelShader* pixelShader)
  {
    BindShaders(vertexShader, nullptr, nullptr, nullptr, pixelShader);
  }

  void GraphicsDeviceContext::BindShaders(VertexShader* vertexShader, GeometryShader* geometryShader, PixelShader* pixelShader)
  {
    BindShaders(vertexShader, nullptr, nullptr, geometryShader, pixelShader);
  }

  void GraphicsDeviceContext::BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, PixelShader* pixelShader)
  {
    BindShaders(vertexShader, hullShader, domainShader, nullptr, pixelShader);
  }

  void GraphicsDeviceContext::BindShaders(VertexShader* vertexShader, HullShader* hullShader, DomainShader* domainShader, GeometryShader* geometryShader, PixelShader* pixelShader)
  {
    if (vertexShader)
    {
      vertexShader->Bind(this);
    }
    else
    {
      _context->VSSetShader(nullptr, nullptr, 0);
    }

    if (hullShader)
    {
      hullShader->Bind(this);
    }
    else
    {
      _context->HSSetShader(nullptr, nullptr, 0);
    }

    if (domainShader)
    {
      domainShader->Bind(this);
    }
    else
    {
      _context->DSSetShader(nullptr, nullptr, 0);
    }

    if (geometryShader)
    {
      geometryShader->Bind(this);
    }
    else
    {
      _context->GSSetShader(nullptr, nullptr, 0);
    }

    if (pixelShader)
    {
      pixelShader->Bind(this);
    }
    else
    {
      _context->PSSetShader(nullptr, nullptr, 0);
    }
  }
}