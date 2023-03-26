#include "pch.h"
#include "GraphicsResource.h"

namespace Axodox::Graphics
{
  GraphicsResource::GraphicsResource(const GraphicsDevice& device) :
    _device(device)
  { }
  
  GraphicsDevice* GraphicsResource::Owner()
  {
    return &_device;
  }

  const GraphicsDevice* GraphicsResource::Owner() const
  {
    return &_device;
  }
}