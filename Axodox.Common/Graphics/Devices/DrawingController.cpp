#include "pch.h"
#include "DrawingController.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::Graphics
{
  DrawingController::DrawingController(const GraphicsDevice& device) :
    GraphicsResource(device)
  {
    //Create draw factory
    {
      D2D1_FACTORY_OPTIONS options;
      zero_memory(options);

#ifdef _DEBUG
      options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

      check_hresult(D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        guid_of<ID2D1Factory6>(),
        &options,
        _drawFactory.put_void()
      ));
    }

    //Create write factory
    check_hresult(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, guid_of<IDWriteFactory2>(), (IUnknown**)_writeFactory.put()));

    //Create device
    {
      com_ptr<IDXGIDevice> dxgiDevice;
      check_hresult(_device->QueryInterface(dxgiDevice.put()));
      check_hresult(_drawFactory->CreateDevice(dxgiDevice.get(), _drawingDevice.put()));
    }

    //Create device context
    {
      check_hresult(_drawingDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, _drawingContext.put()));
      _drawingContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      _drawingContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    }
  }

  ID2D1FactoryT* DrawingController::DrawFactory() const
  {
    return _drawFactory.get();
  }

  IDWriteFactoryT* DrawingController::WriteFactory() const
  {
    return _writeFactory.get();
  }

  ID2D1DeviceT* DrawingController::DrawingDevice() const
  {
    return _drawingDevice.get();
  }

  ID2D1DeviceContextT* DrawingController::DrawingContext() const
  {
    return _drawingContext.get();
  }

  winrt::com_ptr<ID2D1Bitmap1> DrawingController::CreateBitmap(const winrt::com_ptr<ID3D11Texture2D>& texture, uint8_t mip) const
  {
    com_ptr<IDXGISurface2> drawingSurface;
    check_hresult(texture.as<IDXGIResource1>()->CreateSubresourceSurface(mip, drawingSurface.put()));

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
    bitmapProperties.dpiX = 96;
    bitmapProperties.dpiY = 96;
    bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    D3D11_TEXTURE2D_DESC textureDescription;
    texture->GetDesc(&textureDescription);

    D2D1_PIXEL_FORMAT pixelFormat = {};
    pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    pixelFormat.format = textureDescription.Format;

    bitmapProperties.pixelFormat = pixelFormat;

    com_ptr<ID2D1Bitmap1> bitmap;
    check_hresult(_drawingContext->CreateBitmapFromDxgiSurface(drawingSurface.get(), bitmapProperties, bitmap.put()));
    return bitmap;
  }

  DrawingBatch::DrawingBatch(ID2D1DeviceContextT* context) :
    _context(context)
  {
    _context->BeginDraw();
  }

  DrawingBatch::~DrawingBatch()
  {
    _context->EndDraw();
  }

  DrawingBatch::operator ID2D1DeviceContextT* () const
  {
    return _context;
  }

  ID2D1DeviceContextT* DrawingBatch::operator->() const
  {
    return _context;
  }
}