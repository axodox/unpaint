#include "GraphicsResource.h"

namespace Axodox::Graphics
{
  class DrawingController : public GraphicsResource
  {
  public:
    DrawingController(const GraphicsDevice& device);

    ID2D1FactoryT* DrawFactory() const;
    IDWriteFactoryT* WriteFactory() const;

    ID2D1DeviceT* DrawingDevice() const;
    ID2D1DeviceContextT* DrawingContext() const;

    winrt::com_ptr<ID2D1Bitmap1> CreateBitmap(const winrt::com_ptr<ID3D11Texture2D>& texture, uint8_t mip = 0) const;

  private:
    winrt::com_ptr<ID2D1FactoryT> _drawFactory;
    winrt::com_ptr<IDWriteFactoryT> _writeFactory;

    winrt::com_ptr<ID2D1DeviceT> _drawingDevice;
    winrt::com_ptr<ID2D1DeviceContextT> _drawingContext;
  };

  class DrawingBatch
  {
  public:
    explicit DrawingBatch(ID2D1DeviceContextT* context);
    ~DrawingBatch();

    operator ID2D1DeviceContextT* () const;
    ID2D1DeviceContextT* operator->() const;

  private:
    ID2D1DeviceContextT* const _context;
  };
}