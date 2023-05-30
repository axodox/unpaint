#pragma once
#include "pch.h"

namespace Axodox::Graphics
{
  size_t BitsPerPixel(DXGI_FORMAT format);
  bool HasAlpha(DXGI_FORMAT format);

  bool IsUByteN1Compatible(DXGI_FORMAT format);
  bool IsUByteN4Compatible(DXGI_FORMAT format);

  IWICImagingFactory* WicFactory();

  winrt::Windows::Graphics::Imaging::BitmapPixelFormat ToBitmapPixelFormat(DXGI_FORMAT format);
  DXGI_FORMAT ToDxgiFormat(winrt::Windows::Graphics::Imaging::BitmapPixelFormat format);

  WICPixelFormatGUID ToWicPixelFormat(DXGI_FORMAT format);
  DXGI_FORMAT ToDxgiFormat(WICPixelFormatGUID format);
}