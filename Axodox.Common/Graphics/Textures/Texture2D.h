#pragma once
#include "Graphics/Devices/GraphicsResource.h"
#include "Texture2DDefinition.h"
#include "TextureData.h"
#include "Collections/Hasher.h"

namespace Axodox::Graphics
{
  class Texture2D : public GraphicsResource
  {
    struct ResourceViewKey
    {
      DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
      uint32_t Mip = ~0u;
    };

    struct UnorderedViewKey
    {
      DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
      uint32_t Mip = ~0u;
    };

  public:
    Texture2D(const GraphicsDevice& device, const Texture2DDefinition& definition);
    Texture2D(const GraphicsDevice& device, const TextureData& texture);
    Texture2D(const GraphicsDevice& device, const winrt::com_ptr<ID3D11Texture2D>& texture);

    const Texture2DDefinition& Definition() const;

    void BindShaderResourceView(ShaderStage stage, uint32_t slot = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t mip = ~0u, GraphicsDeviceContext* context = nullptr);
    void UnbindShaderResourceView(uint32_t mip = ~0u, GraphicsDeviceContext* context = nullptr);

    void BindUnorderedAccessView(uint32_t slot = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t mip = ~0u, GraphicsDeviceContext* context = nullptr);
    void UnbindUnorderedAccessView(uint32_t mip = ~0u, GraphicsDeviceContext* context = nullptr);

    virtual void Unbind(GraphicsDeviceContext* context = nullptr);

    void Update(const TextureData& texture, uint32_t subresourceIndex = 0, GraphicsDeviceContext* context = nullptr);
    void GenerateMips(GraphicsDeviceContext* context = nullptr);

    void Copy(Texture2D* target, uint32_t mip = ~0u, GraphicsDeviceContext* context = nullptr);

    const winrt::com_ptr<ID3D11Texture2D>& operator*() const;
    ID3D11Texture2D* operator->() const;
    ID3D11Texture2D* get() const;

  protected:
    Texture2DDefinition _definition;
    winrt::com_ptr<ID3D11Texture2D> _texture;

  private:
    std::vector<winrt::com_ptr<ID3D11ShaderResourceView>> _resourceViews;
    std::unordered_map<ResourceViewKey, winrt::com_ptr<ID3D11ShaderResourceView>, Collections::trivial_hasher<ResourceViewKey>, Collections::trivial_comparer<ResourceViewKey>> _alternateResourceViews;

    winrt::com_ptr<ID3D11UnorderedAccessView> _unorderedView;
    std::unordered_map<UnorderedViewKey, winrt::com_ptr<ID3D11UnorderedAccessView>, Collections::trivial_hasher<UnorderedViewKey>, Collections::trivial_comparer<UnorderedViewKey>> _alternateUnorderedViews;

    void InitializeShaderResourceViews(DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool isCube = false, bool isArray = false);
    std::vector<ID3D11ShaderResourceView*> GetResourceViews(ResourceViewKey key);

    void InitializeUnorderedViews(DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool isArray = false);
    ID3D11UnorderedAccessView* GetUnorderedView(UnorderedViewKey key);
  };
}