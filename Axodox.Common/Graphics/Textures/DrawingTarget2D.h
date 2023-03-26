#pragma once
#include "RenderTarget2D.h"
#include "Graphics/Devices/DrawingController.h"

namespace Axodox::Graphics
{
  class DrawingTarget2D : public RenderTarget2D
  {
  public:
    DrawingTarget2D(const DrawingController& drawingController, const Texture2DDefinition& definition);

    ID2D1Bitmap* Bitmap(uint8_t index = 0) const;
    std::span<const winrt::com_ptr<ID2D1Bitmap>> Bitmaps() const;

  private:
    std::vector<winrt::com_ptr<ID2D1Bitmap>> _bitmaps;

    static Texture2DDefinition PreprocessDefinition(Texture2DDefinition definition);
  };
}