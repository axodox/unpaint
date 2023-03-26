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

  protected:
    GraphicsDevice _device;
  };
}