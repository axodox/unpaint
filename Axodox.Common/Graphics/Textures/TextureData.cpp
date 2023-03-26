#include "pch.h"
#include "TextureData.h"
#include "Graphics\Helpers.h"
#include "Storage\ComHelpers.h"

using namespace Axodox::Storage;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  TextureData::TextureData() :
    Width(0),
    Height(0),
    Stride(0),
    Format(DXGI_FORMAT_UNKNOWN)
  { }

  TextureData::TextureData(uint32_t width, uint32_t height, uint32_t stride, DXGI_FORMAT format, Collections::aligned_vector<uint8_t>&& buffer) :
    Width(width),
    Height(height),
    Stride(stride),
    Format(format),
    Buffer(move(buffer))
  { }

  TextureData::TextureData(uint32_t width, uint32_t height, DXGI_FORMAT format) :
    Width(width),
    Height(height),
    Stride((uint32_t)ceil(BitsPerPixel(format) * width / 8.f)),
    Format(format),
    Buffer(Stride * Height)
  { }

  TextureData::TextureData(TextureData&& other) noexcept
  {
    *this = move(other);
  }

  void TextureData::operator=(TextureData&& other) noexcept
  {
    Width = other.Width;
    Height = other.Height;
    Stride = other.Stride;
    Format = other.Format;
    Buffer = move(other.Buffer);

    other.Width = 0;
    other.Height = 0;
    other.Stride = 0;
    other.Format = DXGI_FORMAT_UNKNOWN;
  }
  
  uint32_t TextureData::MainDimension() const
  {
    return max(Width, Height);
  }

  uint32_t TextureData::PixelCount() const
  {
    return Width * Height;
  }

  uint32_t TextureData::ByteCount() const
  {
    return Stride * Height;
  }

  bool TextureData::IsValid() const
  {
    return
      Width > 0u && Height > 0u &&
      Stride * 8u >= Width * BitsPerPixel(Format) &&
      Buffer.size() == Stride * Height;
  }

  TextureData::operator bool() const
  {
    return !Buffer.empty();
  }

  TextureData TextureData::FromBuffer(std::span<const uint8_t> buffer)
  {
    auto wicFactory = WicFactory();

    //Load data to WIC bitmap
    com_ptr<IWICBitmapSource> bitmap;
    {
      auto stream = to_stream(buffer);

      com_ptr<IWICBitmapDecoder> bitmapDecoder;
      check_hresult(wicFactory->CreateDecoderFromStream(
        stream.get(),
        NULL,
        WICDecodeMetadataCacheOnDemand,
        bitmapDecoder.put()
      ));

      check_hresult(bitmapDecoder->GetFrame(0, reinterpret_cast<IWICBitmapFrameDecode**>(bitmap.put())));
    }

    //Covert data to BGRA
    {
      com_ptr<IWICFormatConverter> converter;
      check_hresult(wicFactory->CreateFormatConverter(converter.put()));
      check_hresult(converter->Initialize(
        bitmap.get(), GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.f,
        WICBitmapPaletteTypeMedianCut));
      bitmap = move(converter);
    }

    //Define result
    TextureData result;
    check_hresult(bitmap->GetSize(&result.Width, &result.Height));
    result.Stride = result.Width * 4;
    result.Buffer.resize(result.ByteCount());
    result.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    WICRect rect = { 0, 0, int32_t(result.Width), int32_t(result.Height) };
    check_hresult(bitmap->CopyPixels(&rect, result.Stride, uint32_t(result.Buffer.size()), result.Buffer.data()));

    return result;
  }
}