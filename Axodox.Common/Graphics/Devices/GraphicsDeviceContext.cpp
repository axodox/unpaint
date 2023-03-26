#include "pch.h"
#include "GraphicsDeviceContext.h"

#include "Graphics/Shaders/VertexShader.h"
#include "Graphics/Shaders/HullShader.h"
#include "Graphics/Shaders/DomainShader.h"
#include "Graphics/Shaders/GeometryShader.h"
#include "Graphics/Shaders/PixelShader.h"

#include "Graphics/Textures/RenderTarget2D.h"
#include "Graphics/Textures/DepthStencil2D.h"

using namespace std;

namespace Axodox::Graphics
{
  GraphicsDeviceContext::GraphicsDeviceContext(const winrt::com_ptr<ID3D11DeviceContextT>& context) :
    _context(context)
  {
    _boundRenderTargetViews.fill(nullptr);
  }

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
    _boundShaderResourceViews[stage][slot] = view;

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

  void GraphicsDeviceContext::UnbindShaderResourceView(ID3D11ShaderResourceView* view)
  {
    if (!view) return;

    for (auto& [shaderStage, shaderResourceViews] : _boundShaderResourceViews)
    {
      for (auto& [slot, shaderResourceView] : shaderResourceViews)
      {
        if (shaderResourceView == view)
        {
          BindShaderResourceView(nullptr, shaderStage, slot);
        }
      }
    }
  }

  void GraphicsDeviceContext::BindUnorderedAccessView(ID3D11UnorderedAccessView* view, uint32_t slot)
  {
    _boundUnorderedAccessViews[slot] = view;

    auto uavInitialCount = 0u;
    _context->CSSetUnorderedAccessViews(slot, 1, &view, &uavInitialCount);
  }

  void GraphicsDeviceContext::UnbindUnorderedAccessView(ID3D11UnorderedAccessView* view)
  {
    if (!view) return;

    for (auto& [slot, unorderedAccessView] : _boundUnorderedAccessViews)
    {
      if (unorderedAccessView == view)
      {
        BindUnorderedAccessView(nullptr, slot);
      }
    }
  }

  void GraphicsDeviceContext::BindRenderTargetView(ID3D11RenderTargetView* view)
  {
    _boundRenderTargetViews.fill(nullptr);
    _boundRenderTargetViews[0] = view;
    _boundDepthStencilView = nullptr;

    _context->OMSetRenderTargets(uint32_t(_boundRenderTargetViews.size()), _boundRenderTargetViews.data(), _boundDepthStencilView);
  }

  void GraphicsDeviceContext::UnbindRenderTargetView(ID3D11RenderTargetView* view)
  {
    bool changed = false;
    for (auto& boundView : _boundRenderTargetViews)
    {
      if (boundView == view)
      {
        boundView = nullptr;
        changed = true;
      }
    }

    if (changed)
    {
      _context->OMSetRenderTargets(uint32_t(_boundRenderTargetViews.size()), _boundRenderTargetViews.data(), _boundDepthStencilView);
    }
  }

  void GraphicsDeviceContext::BindDepthStencilView(ID3D11DepthStencilView* view)
  {
    _boundRenderTargetViews.fill(nullptr);
    _boundDepthStencilView = view;

    _context->OMSetRenderTargets(uint32_t(_boundRenderTargetViews.size()), _boundRenderTargetViews.data(), _boundDepthStencilView);
  }

  void GraphicsDeviceContext::UnbindDepthStencilView(ID3D11DepthStencilView* view)
  {
    if (view != _boundDepthStencilView) return;

    _boundDepthStencilView = nullptr;
    _context->OMSetRenderTargets(uint32_t(_boundRenderTargetViews.size()), _boundRenderTargetViews.data(), _boundDepthStencilView);
  }

  void GraphicsDeviceContext::BindRenderTargets(std::span<RenderTarget2D> renderTargets, DepthStencil2D* depthStencil)
  {
    _boundRenderTargetViews.fill(nullptr);
    for (auto index = 0; auto& renderTarget : renderTargets)
    {
      _boundRenderTargetViews[index++] = renderTarget.get();
    }
    _boundDepthStencilView = depthStencil ? depthStencil->get() : nullptr;

    _context->OMSetRenderTargets(uint32_t(_boundRenderTargetViews.size()), _boundRenderTargetViews.data(), _boundDepthStencilView);
    _context->RSSetViewports(1, &renderTargets[0].Viewport());
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