#pragma once
#include "VertexDefinitions.h"
#include "Graphics\Devices\GraphicsResource.h"

namespace Axodox::Graphics
{
  class Mesh : public GraphicsResource
  {
  public:
    using GraphicsResource::GraphicsResource;
    virtual ~Mesh() = default;

    virtual void Draw(GraphicsDeviceContext* context = nullptr) = 0;
    virtual void DrawInstanced(uint32_t instanceCount, GraphicsDeviceContext* context = nullptr) = 0;

    virtual VertexDefition Defition() const = 0;
  };
}