#pragma once
#include "GraphicsDevice.h"

namespace Axodox::Graphics
{
  class GraphicsResource
  {
  public:
    GraphicsResource(const GraphicsDevice& device);

    virtual ~GraphicsResource() = default;

  protected:
    GraphicsDevice _device;
  };
}