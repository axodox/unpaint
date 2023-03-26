#pragma once
#include "pch.h"

namespace Axodox::Graphics
{
  size_t BitsPerPixel(DXGI_FORMAT format);

  IWICImagingFactory* WicFactory();
}