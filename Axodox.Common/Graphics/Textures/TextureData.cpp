#include "pch.h"
#include "TextureData.h"
#include "Graphics/Helpers.h"
#include "Storage/ComHelpers.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/Win32.h"

using namespace Axodox::Infrastructure;
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

  TextureData TextureData::FromBuffer(std::span<const uint8_t> buffer, TextureImageFormat format, std::string* metadata)
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
      
      if (metadata)
      {
        com_ptr<IWICMetadataQueryReader> metadataQueryReader;
        check_hresult(reinterpret_cast<IWICBitmapFrameDecode*>(wicBitmap.get())->GetMetadataQueryReader(metadataQueryReader.put()));

        PROPVARIANT metadataProperty;
        zero_memory(metadataProperty);
        metadataQueryReader->GetMetadataByName(L"/tEXt/{str=Metadata}", &metadataProperty);

        if (metadataProperty.vt == VT_LPSTR)
        {
          *metadata = metadataProperty.pszVal;
        }
      }
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

  std::vector<uint8_t> TextureData::ToBuffer(std::string_view metadata) const
  {
    if (Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB && Format != DXGI_FORMAT_B8G8R8A8_UNORM) throw bad_cast();

    auto wicFactory = WicFactory();

    //Create stream
    com_ptr<IStream> stream;
    check_hresult(CreateStreamOnHGlobal(nullptr, true, stream.put()));

    com_ptr<IWICStream> wicStream;
    check_hresult(wicFactory->CreateStream(wicStream.put()));
    check_hresult(wicStream->InitializeFromIStream(stream.get()));

    //Create frame encoder
    com_ptr<IWICBitmapEncoder> wicBitmapEncoder;
    check_hresult(wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, wicBitmapEncoder.put()));

    wicBitmapEncoder->Initialize(wicStream.get(), WICBitmapEncoderNoCache);

    //Initalize frame
    com_ptr<IWICBitmapFrameEncode> wicBitmapFrameEncode;
    check_hresult(wicBitmapEncoder->CreateNewFrame(wicBitmapFrameEncode.put(), nullptr));

    auto pixelFormat = GUID_WICPixelFormat32bppBGRA;
    check_hresult(wicBitmapFrameEncode->Initialize(nullptr));
    check_hresult(wicBitmapFrameEncode->SetPixelFormat(&pixelFormat));
    check_hresult(wicBitmapFrameEncode->SetSize(Width, Height));

    //Encode metadata
    if (!metadata.empty())
    {
      com_ptr<IWICMetadataQueryWriter> metadataQueryWriter;
      check_hresult(wicBitmapFrameEncode->GetMetadataQueryWriter(metadataQueryWriter.put()));

      PROPVARIANT textEntryProperty;
      PropVariantInit(&textEntryProperty);
      textEntryProperty.vt = VT_LPSTR;
      textEntryProperty.pszVal = const_cast<char*>(metadata.data());

      check_hresult(metadataQueryWriter->SetMetadataByName(L"/tEXt/{str=Metadata}", &textEntryProperty));
    }

    //Write pixels
    check_hresult(wicBitmapFrameEncode->WritePixels(Height, Stride, ByteCount(), const_cast<uint8_t*>(Buffer.data())));
            
    //Close frame
    check_hresult(wicBitmapFrameEncode->Commit());
    check_hresult(wicBitmapEncoder->Commit());

    //Return result
    STATSTG streamStats;
    check_hresult(stream->Stat(&streamStats, STATFLAG_DEFAULT));

    vector<uint8_t> result;
    result.resize(size_t(streamStats.cbSize.QuadPart));

    check_hresult(stream->Seek(LARGE_INTEGER{ 0 }, STREAM_SEEK_SET, nullptr));
    check_hresult(stream->Read(result.data(), ULONG(result.size()), nullptr));

    return result;
  }

  TextureData TextureData::FromWicBitmap(const winrt::com_ptr<IWICBitmap>& wicBitmap)
  {
    WICPixelFormatGUID format;
    check_hresult(wicBitmap->GetPixelFormat(&format));

    if (format != GUID_WICPixelFormat32bppBGRA) throw bad_cast();

    uint32_t width, height;
    check_hresult(wicBitmap->GetSize(&width, &height));
    
    WICRect wicRect{
      .X = 0,
      .Y = 0,
      .Width = int32_t(width),
      .Height = int32_t(height)
    };

    com_ptr<IWICBitmapLock> wicBitmapLock;
    check_hresult(wicBitmap->Lock(&wicRect, WICBitmapLockRead, wicBitmapLock.put()));

    uint32_t bufferSize;
    uint32_t bufferStride;
    uint8_t* bufferData;
    check_hresult(wicBitmapLock->GetDataPointer(&bufferSize, &bufferData));
    check_hresult(wicBitmapLock->GetStride(&bufferStride));

    TextureData result{ width, height, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
    for (uint32_t row = 0; row < height; row++)
    {
      auto pSource = reinterpret_cast<uint32_t*>(bufferData + bufferStride * row);
      auto pTarget = result.Row<uint32_t>(row);      
      memcpy(pTarget, pSource, width * sizeof(uint32_t));
    }

    return result;
  }

  winrt::com_ptr<IWICBitmap> TextureData::ToWicBitmap() const
  {
    if (Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB && Format != DXGI_FORMAT_B8G8R8A8_UNORM) throw bad_cast();

    auto wicFactory = WicFactory();

    com_ptr<IWICBitmap> wicBitmap;
    check_hresult(wicFactory->CreateBitmap(Width, Height, GUID_WICPixelFormat32bppBGRA, WICBitmapNoCache, wicBitmap.put()));

    WICRect wicRect{
      .X = 0,
      .Y = 0,
      .Width = int32_t(Width),
      .Height = int32_t(Height)
    };

    com_ptr<IWICBitmapLock> wicBitmapLock;
    check_hresult(wicBitmap->Lock(&wicRect, WICBitmapLockWrite, wicBitmapLock.put()));

    uint32_t bufferSize;
    uint32_t bufferStride;
    uint8_t* bufferData;
    check_hresult(wicBitmapLock->GetDataPointer(&bufferSize, &bufferData));
    check_hresult(wicBitmapLock->GetStride(&bufferStride));

    for (uint32_t row = 0; row < Height; row++)
    {
      auto pSource = Row<uint32_t>(row);
      auto pTarget = reinterpret_cast<uint32_t*>(bufferData + bufferStride * row);
      memcpy(pTarget, pSource, Width * sizeof(uint32_t));
    }

    return wicBitmap;
  }

  TextureData TextureData::Resize(uint32_t width, uint32_t height) const
  {
    if (width == Width && height == Height) return TextureData(*this);

    auto wicFactory = WicFactory();
    auto wicBitmap = ToWicBitmap();

    com_ptr<IWICBitmapScaler> wicBitmapScaler;
    check_hresult(wicFactory->CreateBitmapScaler(wicBitmapScaler.put()));
    check_hresult(wicBitmapScaler->Initialize(wicBitmap.get(), width, height, WICBitmapInterpolationModeHighQualityCubic));

    return TextureData::FromWicBitmap(wicBitmapScaler.as<IWICBitmap>());
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