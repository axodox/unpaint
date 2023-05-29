#include "pch.h"
#include "TextureData.h"
#include "Graphics/Helpers.h"
#include "Storage/ComHelpers.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/Win32.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace DirectX;
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

  enum class JpegImageOrientation : uint16_t
  {
    Unknown = 0,
    Normal = 1,
    FlipHorizontal = 2,
    Rotate180 = 3,
    FlipVertical = 4,
    FlipHorizontalRotate270 = 5,
    Rotate90 = 6,
    FlipHorizontalRotate90 = 7,
    Rotate270 = 8
  };

  TextureData TextureData::FromBuffer(std::span<const uint8_t> buffer, TextureImageFormat format, std::string* metadata)
  {
    if (buffer.empty()) return {};

    auto wicFactory = WicFactory();

    //Load data to WIC bitmap
    auto tranformation = WICBitmapTransformRotate0;
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
      
      //Get container format
      GUID containerFormat;
      check_hresult(wicBitmapDecoder->GetContainerFormat(&containerFormat));

      //Read metadata
      com_ptr<IWICMetadataQueryReader> metadataQueryReader;
      check_hresult(reinterpret_cast<IWICBitmapFrameDecode*>(wicBitmap.get())->GetMetadataQueryReader(metadataQueryReader.put()));

      //Parse generation metadata
      if (metadata)
      {
        PROPVARIANT metadataProperty;
        zero_memory(metadataProperty);
        metadataQueryReader->GetMetadataByName(L"/tEXt/{str=Metadata}", &metadataProperty);

        if (metadataProperty.vt == VT_LPSTR)
        {
          *metadata = metadataProperty.pszVal;
        }
      }

      //Read orientation metadata
      {
        if (containerFormat == GUID_ContainerFormatJpeg)
        {
          PROPVARIANT metadataProperty;
          zero_memory(metadataProperty);
          metadataQueryReader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &metadataProperty);

          if (metadataProperty.vt == VT_UI2)
          {
            JpegImageOrientation orientation{ metadataProperty.uiVal };

            switch (orientation)
            {
            case JpegImageOrientation::FlipHorizontal:
              tranformation = WICBitmapTransformFlipHorizontal;
              break;
            case JpegImageOrientation::Rotate180:
              tranformation = WICBitmapTransformRotate180;
              break;
            case JpegImageOrientation::FlipVertical:
              tranformation = WICBitmapTransformFlipVertical;
              break;
            case JpegImageOrientation::FlipHorizontalRotate270:
              tranformation = bitwise_or(WICBitmapTransformFlipHorizontal, WICBitmapTransformRotate270);
              break;
            case JpegImageOrientation::Rotate90:
              tranformation = WICBitmapTransformRotate90;
              break;
            case JpegImageOrientation::FlipHorizontalRotate90:
              tranformation = bitwise_or(WICBitmapTransformFlipHorizontal, WICBitmapTransformRotate90);
              break;
            case JpegImageOrientation::Rotate270:
              tranformation = WICBitmapTransformRotate270;
              break;
            }
          }
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

    //Rotate & flip
    if (tranformation != WICBitmapTransformRotate0)
    {
      com_ptr<IWICBitmapFlipRotator> wicFlipRotator;
      check_hresult(wicFactory->CreateBitmapFlipRotator(wicFlipRotator.put()));

      check_hresult(wicFlipRotator->Initialize(wicBitmap.get(), tranformation));
      wicBitmap = move(wicFlipRotator);
    }

    //Define result
    TextureData result;
    check_hresult(wicBitmap->GetSize(&result.Width, &result.Height));
    result.Stride = result.Width * uint32_t(BitsPerPixel(dxgiFormat)) / 8;
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

  TextureData TextureData::FromWicBitmap(const winrt::com_ptr<IWICBitmapSource>& wicBitmap)
  {
    WICPixelFormatGUID format;
    check_hresult(wicBitmap->GetPixelFormat(&format));

    uint32_t width, height;
    check_hresult(wicBitmap->GetSize(&width, &height));
    
    WICRect wicRect{
      .X = 0,
      .Y = 0,
      .Width = int32_t(width),
      .Height = int32_t(height)
    };

    TextureData result{ width, height, ToDxgiFormat(format)};
    check_hresult(wicBitmap->CopyPixels(&wicRect, result.Stride, result.ByteCount(), result.Buffer.data()));

    return result;
  }

  winrt::com_ptr<IWICBitmap> TextureData::ToWicBitmap() const
  {
    auto wicFactory = WicFactory();

    com_ptr<IWICBitmap> wicBitmap;
    check_hresult(wicFactory->CreateBitmap(Width, Height, ToWicPixelFormat(Format), WICBitmapCacheOnDemand, wicBitmap.put()));

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

    auto dataStride = Width * BitsPerPixel(Format) / 8;
    for (uint32_t row = 0; row < Height; row++)
    {
      auto pSource = Row<uint32_t>(row);
      auto pTarget = reinterpret_cast<uint32_t*>(bufferData + bufferStride * row);
      memcpy(pTarget, pSource, dataStride);
    }

    return wicBitmap;
  }

  TextureData TextureData::FromSoftwareBitmap(const winrt::Windows::Graphics::Imaging::SoftwareBitmap& softwareBitmap)
  {
    if (!softwareBitmap) return {};

    auto bitmapData = softwareBitmap.LockBuffer(BitmapBufferAccessMode::Read);
    auto byteAccess = bitmapData.CreateReference().as<IMemoryBufferByteAccess>();
    auto plane = bitmapData.GetPlaneDescription(0);

    TextureData result{ uint32_t(plane.Width), uint32_t(plane.Height), ToDxgiFormat(softwareBitmap.BitmapPixelFormat())};
    auto output = result.Buffer.data();

    uint32_t capacity;
    uint8_t* input;
    check_hresult(byteAccess->GetBuffer(&input, &capacity));

    for (auto i = 0u; i < result.Height; i++)
    {
      memcpy(output + i * result.Stride, input + i * plane.Stride, result.Stride);
    }

    return result;
  }

  winrt::Windows::Graphics::Imaging::SoftwareBitmap TextureData::ToSoftwareBitmap() const
  {
    if (!*this) return nullptr;

    SoftwareBitmap result{ ToBitmapPixelFormat(Format), int32_t(Width), int32_t(Height), HasAlpha(Format) ? BitmapAlphaMode::Premultiplied : BitmapAlphaMode::Ignore };

    auto input = Buffer.data();
    auto bitmapData = result.LockBuffer(BitmapBufferAccessMode::Write);
    auto byteAccess = bitmapData.CreateReference().as<IMemoryBufferByteAccess>();
    auto plane = bitmapData.GetPlaneDescription(0);

    uint32_t capacity;
    uint8_t* output;
    check_hresult(byteAccess->GetBuffer(&output, &capacity));

    for (auto i = 0u; i < Height; i++)
    {
      memcpy(output + i * plane.Stride, input + i * Stride, Stride);
    }

    return result;
  }

  TextureData TextureData::Resize(uint32_t width, uint32_t height) const
  {
    if (width == Width && height == Height) return TextureData(*this);

    auto wicFactory = WicFactory();
    auto wicBitmap = ToWicBitmap();

    com_ptr<IWICBitmapScaler> wicBitmapScaler;
    check_hresult(wicFactory->CreateBitmapScaler(wicBitmapScaler.put()));
    check_hresult(wicBitmapScaler->Initialize(wicBitmap.get(), width, height, WICBitmapInterpolationModeHighQualityCubic));

    return TextureData::FromWicBitmap(wicBitmapScaler);
  }

  TextureData TextureData::UniformResize(uint32_t width, uint32_t height, Rect* sourceRect) const
  {
    auto sourceAspectRatio = float(Width) / float(Height);
    auto targetAspectRatio = float(width) / float(height);

    uint32_t uniformWidth, uniformHeight;
    if (targetAspectRatio > sourceAspectRatio)
    {
      uniformWidth = uint32_t(width * sourceAspectRatio / targetAspectRatio);
      uniformHeight = height;
    }
    else
    {
      uniformWidth = width;
      uniformHeight = uint32_t(height * targetAspectRatio / sourceAspectRatio);
    }

    auto resizedTexture = Resize(uniformWidth, uniformHeight);

    Rect rect;
    if (width != uniformWidth || height != uniformHeight)
    {
      if (targetAspectRatio > sourceAspectRatio)
      {
        auto center = width / 2;
        auto halfWidth = resizedTexture.Width / 2;
        rect = { int32_t(center - halfWidth), 0, int32_t(center + halfWidth), int32_t(resizedTexture.Height) };

        resizedTexture = resizedTexture.ExtendHorizontally(width);
      }
      else
      {
        auto center = height / 2;
        auto halfHeight = resizedTexture.Height / 2;
        rect = { 0, int32_t(center - halfHeight), int32_t(resizedTexture.Width), int32_t(center + halfHeight) };

        resizedTexture = resizedTexture.ExtendVertically(height);
      }
    }
    else
    {
      rect = { 0, 0, int32_t(width), int32_t(height) };
    }

    if (sourceRect) *sourceRect = rect;


    return resizedTexture;
  }

  TextureData TextureData::ExtendHorizontally(uint32_t width) const
  {
    if (width <= Width) return TextureData{ *this };

    TextureData result{ width, Height, Format };

    auto bytesPerPixel = BitsPerPixel(Format) / 8;
    auto offset = bytesPerPixel * ((width - Width) / 2);
    for (uint32_t row = 0; row < Height; row++)
    {
      auto pSource = Buffer.data() + row * Stride;
      auto pTarget = result.Buffer.data() + row * result.Stride + offset;
      memcpy(pTarget, pSource, Stride);
    }

    return result;
  }

  TextureData TextureData::ExtendVertically(uint32_t height) const
  {
    if (height <= Height) return TextureData{ *this };

    TextureData result{ Width, height, Format };

    auto offset = Stride * ((height - Height) / 2);
    for (uint32_t row = 0; row < Height; row++)
    {
      auto pSource = Buffer.data() + row * Stride;
      auto pTarget = result.Buffer.data() + row * result.Stride + offset;
      memcpy(pTarget, pSource, Stride);
    }

    return result;
  }

  TextureData TextureData::TruncateHorizontally(uint32_t width) const
  {
    if (width >= Width) return TextureData{ *this };

    TextureData result{ width, Height, Format };

    auto bytesPerPixel = BitsPerPixel(Format) / 8;
    auto offset = bytesPerPixel * ((Width - width) / 2);
    for (uint32_t row = 0; row < Height; row++)
    {
      auto pSource = Buffer.data() + row * Stride + offset;
      auto pTarget = result.Buffer.data() + row * result.Stride;
      memcpy(pTarget, pSource, result.Stride);
    }

    return result;
  }

  TextureData TextureData::TruncateVertically(uint32_t height) const
  {
    if (height >= Height) return TextureData{ *this };

    TextureData result{ Width, height, Format };

    auto offset = Stride * ((Height - height) / 2);
    for (uint32_t row = 0; row < height; row++)
    {
      auto pSource = Buffer.data() + row * Stride + offset;
      auto pTarget = result.Buffer.data() + row * result.Stride;
      memcpy(pTarget, pSource, Stride);
    }

    return result;
  }

  Size TextureData::Size() const
  {
    return { int32_t(Width), int32_t(Height) };
  }

  Rect TextureData::FindNonZeroRect() const
  {
    XMUINT4 zero{ 0, 0, 0, 0 };

    auto result = Rect::Empty;
    auto elementSize = BitsPerPixel(Format) / 8;
    for (auto y = 0u; y < Height; y++)
    {
      auto pixel = Buffer.data() + Stride * y;

      for (auto x = 0u; x < Width; x++)
      {
        if (memcmp(pixel++, &zero, elementSize) != 0)
        {
          result = result.Extend(Point{ int32_t(x), int32_t(y) });
        }
      }
    }

    return result;
  }

  TextureData TextureData::GetTexture(Rect rect) const
  {
    rect = rect.Clamp(Size());

    auto size = rect.Size();
    TextureData result{ uint32_t(size.Width), uint32_t(size.Height), Format };

    auto elementSize = BitsPerPixel(Format) / 8;
    for (auto y = rect.Top; y < rect.Bottom; y++)
    {
      auto source = Buffer.data() + Stride * y + rect.Left * elementSize;
      auto target = result.Buffer.data() + result.Stride * (y - rect.Top);
      memcpy(target, source, result.Stride);
    }

    return result;
  }

  TextureData TextureData::MergeTexture(const TextureData& texture, Point position)
  {
    auto targetRect = Rect::FromLeftTopSize(position, texture.Size());
    if (!Rect::FromSize(Size()).Contains(targetRect)) throw out_of_range("The specified texture does not fit into the current one.");

    auto result{ *this };    
    auto elementSize = BitsPerPixel(Format) / 8;
    for (auto y = targetRect.Top; y < targetRect.Bottom; y++)
    {
      auto source = texture.Buffer.data() + texture.Stride * (y - targetRect.Top);
      auto target = result.Buffer.data() + result.Stride * y + targetRect.Left * elementSize;
      memcpy(target, source, texture.Stride);
    }

    return result;
  }
}