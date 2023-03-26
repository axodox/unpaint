#pragma once
#include "GraphicsDevice.h"

namespace Axodox::Graphics
{
  class GraphicsResource
  {
  public:
    GraphicsResource(const GraphicsDevice& device);
    virtual ~GraphicsResource() = default;

    GraphicsDevice* Owner();
    const GraphicsDevice* Owner() const;

  protected:
    GraphicsDevice _device;
  };
}