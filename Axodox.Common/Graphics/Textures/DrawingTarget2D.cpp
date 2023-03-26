#include "pch.h"
#include "DrawingTarget2D.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;

namespace Axodox::Graphics
{
  DrawingTarget2D::DrawingTarget2D(const DrawingController& drawingController, const Texture2DDefinition& definition) :
    RenderTarget2D(*drawingController.Owner(), PreprocessDefinition(definition))
  {
    for (uint8_t i = 0u; i < definition.MipCount; i++)
    {
      _bitmaps.push_back(drawingController.CreateBitmap(_texture, i));
    }
  }

  ID2D1Bitmap* DrawingTarget2D::Bitmap(uint8_t index) const
  {
    return _bitmaps.at(index).get();
  }

  std::span<const winrt::com_ptr<ID2D1Bitmap>> DrawingTarget2D::Bitmaps() const
  {
    return _bitmaps;
  }

  Texture2DDefinition DrawingTarget2D::PreprocessDefinition(Texture2DDefinition definition)
  {
    set_flag(definition.Flags, Texture2DFlags::GenerateMips, definition.MipCount > 1u);
    return definition;
  }
}