#pragma once
#include "SimpleMesh.h"
#include "Graphics\Buffers\IndexBuffer.h"

namespace Axodox::Graphics
{
  template<typename TVertex, typename TIndex>
  struct IndexedMeshDescription
  {
    std::vector<TVertex> Vertices;
    std::vector<TIndex> Indices;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
  };

  class IndexedMesh : public SimpleMesh
  {
  public:
    template<typename TVertex, typename TIndex>
    IndexedMesh(const GraphicsDevice& device, TypedCapacityOrImmutableData<TVertex> vertexSource, TypedCapacityOrImmutableData<TIndex> indexSource, D3D11_PRIMITIVE_TOPOLOGY topology) :
      SimpleMesh(device, vertexSource, topology),
      _indexBuffer(device, indexSource)
    { }

    template<typename TVertex, typename TIndex>
    IndexedMesh(const GraphicsDevice& device, const IndexedMeshDescription<TVertex, TIndex>& description) :
      IndexedMesh(device, description.Vertices, description.Indices, description.Topology)
    { }

    virtual void Draw(GraphicsDeviceContext* context = nullptr) override;
    virtual void DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context = nullptr) override;

    uint32_t IndexCapacity() const;

    template<typename TVertex, typename TIndex>
    void Upload(std::span<const TVertex> vertices, std::span<const TIndex> indices, GraphicsDeviceContext* context = nullptr)
    {
      _vertexBuffer.Upload(vertices, context);
      _indexBuffer.Upload(indices, context);
    }

  private:
    IndexBuffer _indexBuffer;
  };
}