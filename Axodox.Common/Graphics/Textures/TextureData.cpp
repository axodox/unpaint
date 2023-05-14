#include "pch.h"
#include "TextureData.h"
#include "Graphics/Helpers.h"
#include "Storage/ComHelpers.h"

using namespace Axodox::Storage;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Graphics::Imaging;
using namespace ::Windows::Foundation;

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
    Stride((uint32_t)ceil(BitsPerPixel(format)* width / 8.f)),
    Format(format),
    Buffer(Stride* Height)
  { }

  TextureData::TextureData(TextureData&& other) noexcept
  {
    *this = move(other);
  }

  TextureData& TextureData::operator=(TextureData&& other) noexcept
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

    return *this;
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

  TextureData TextureData::FromBuffer(std::span<const uint8_t> buffer, TextureImageFormat format)
  {
    auto wicFactory = WicFactory();

    //Load data to WIC bitmap
    com_ptr<IWICBitmapSource> wicBitmap;
    {
      auto stream = to_stream(buffer);

      com_ptr<IWICBitmapDecoder> wicBitmapDecoder;
      check_hresult(wicFactory->CreateDecoderFromStream(
        stream.get(),
        NULL,
        WICDecodeMetadataCacheOnDemand,
        wicBitmapDecoder.put()
      ));

      check_hresult(wicBitmapDecoder->GetFrame(0, reinterpret_cast<IWICBitmapFrameDecode**>(wicBitmap.put())));
    }

    //Select format
    WICPixelFormatGUID wicFormat;
    DXGI_FORMAT dxgiFormat;
    switch (format)
    {
    case TextureImageFormat::Rgba8:
      wicFormat = GUID_WICPixelFormat32bppBGRA;
      dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      break;
    case TextureImageFormat::Gray8:
      wicFormat = GUID_WICPixelFormat8bppGray;
      dxgiFormat = DXGI_FORMAT_R8_UNORM;
      break;
    default:
      throw logic_error("Texture image format not implemented.");
    }

    //Covert data to target format
    {
      com_ptr<IWICFormatConverter> wicFormatConverter;
      check_hresult(wicFactory->CreateFormatConverter(wicFormatConverter.put()));
      check_hresult(wicFormatConverter->Initialize(
        wicBitmap.get(), wicFormat,
        WICBitmapDitherTypeNone, nullptr, 0.f,
        WICBitmapPaletteTypeMedianCut));
      wicBitmap = move(wicFormatConverter);
    }

    //Define result
    TextureData result;
    check_hresult(wicBitmap->GetSize(&result.Width, &result.Height));
    result.Stride = result.Width * uint32_t(BitsPerPixel(dxgiFormat));
    result.Buffer.resize(result.ByteCount());
    result.Format = dxgiFormat;

    WICRect rect = { 0, 0, int32_t(result.Width), int32_t(result.Height) };
    check_hresult(wicBitmap->CopyPixels(&rect, result.Stride, uint32_t(result.Buffer.size()), result.Buffer.data()));

    return result;
  }

  std::vector<uint8_t> TextureData::ToBuffer() const
  {
    if (Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB && Format != DXGI_FORMAT_B8G8R8A8_UNORM) throw bad_cast();

    auto wicFactory = WicFactory();

    com_ptr<IStream> stream;
    check_hresult(CreateStreamOnHGlobal(nullptr, true, stream.put()));

    com_ptr<IWICStream> wicStream;
    check_hresult(wicFactory->CreateStream(wicStream.put()));
    check_hresult(wicStream->InitializeFromIStream(stream.get()));

    com_ptr<IWICBitmapEncoder> wicBitmapEncoder;
    check_hresult(wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, wicBitmapEncoder.put()));

    wicBitmapEncoder->Initialize(wicStream.get(), WICBitmapEncoderNoCache);

    com_ptr<IWICBitmapFrameEncode> wicBitmapFrameEncode;
    check_hresult(wicBitmapEncoder->CreateNewFrame(wicBitmapFrameEncode.put(), nullptr));

    auto pixelFormat = GUID_WICPixelFormat32bppBGRA;
    check_hresult(wicBitmapFrameEncode->Initialize(nullptr));
    check_hresult(wicBitmapFrameEncode->SetPixelFormat(&pixelFormat));
    check_hresult(wicBitmapFrameEncode->SetSize(Width, Height));
    check_hresult(wicBitmapFrameEncode->WritePixels(Height, Stride, ByteCount(), const_cast<uint8_t*>(Buffer.data())));
    check_hresult(wicBitmapFrameEncode->Commit());

    check_hresult(wicBitmapEncoder->Commit());

    STATSTG streamStats;
    check_hresult(stream->Stat(&streamStats, STATFLAG_DEFAULT));

    vector<uint8_t> result;
    result.resize(size_t(streamStats.cbSize.QuadPart));

    check_hresult(stream->Seek(LARGE_INTEGER{ 0 }, STREAM_SEEK_SET, nullptr));
    check_hresult(stream->Read(result.data(), ULONG(result.size()), nullptr));

    return result;
  }

  winrt::Windows::Graphics::Imaging::SoftwareBitmap TextureData::ToSoftwareBitmap() const
  {
    if (Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB && Format != DXGI_FORMAT_B8G8R8A8_UNORM) throw bad_cast();

    SoftwareBitmap bitmap{ BitmapPixelFormat::Bgra8, int32_t(Width), int32_t(Height), BitmapAlphaMode::Premultiplied };

    auto input = Buffer.data();
    auto bitmapData = bitmap.LockBuffer(BitmapBufferAccessMode::Write);
    auto byteAccess = bitmapData.CreateReference().as<IMemoryBufferByteAccess>();

    uint32_t capacity;
    uint8_t* output;
    check_hresult(byteAccess->GetBuffer(&output, &capacity));
    memcpy(output, input, min(capacity, ByteCount()));

    return bitmap;
  }
}