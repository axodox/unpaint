#include "pch.h"
#include "Rect.h"

using namespace std;

namespace Axodox::Graphics
{
  const Rect Rect::Empty = {
    numeric_limits<int32_t>::max(),
    numeric_limits<int32_t>::max(),
    numeric_limits<int32_t>::min(),
    numeric_limits<int32_t>::min()
  };

  Rect Rect::FromSize(Graphics::Size size)
  {
    return {
      0,
      0,
      size.Width,
      size.Height
    };
  }

  Rect Rect::FromCenterSize(Point center, Graphics::Size size)
  {
    auto halfSize = size.Half();
    return { 
      center.X - halfSize.Width,
      center.Y - halfSize.Height,
      center.X + halfSize.Width,
      center.Y + halfSize.Height
    };
  }

  Rect Rect::FromLeftTopSize(Point leftTop, Graphics::Size size)
  {
    return {
      leftTop.X,
      leftTop.Y,
      leftTop.X + size.Width,
      leftTop.Y + size.Height
    };
  }

  Rect Rect::Extend(Point point) const
  {
    auto result{ *this };

    if (point.X < result.Left) result.Left = point.X;
    if (point.Y < result.Top) result.Top = point.Y;
    if (point.X + 1 > result.Right) result.Right = point.X + 1;
    if (point.Y + 1 > result.Bottom) result.Bottom = point.Y + 1;

    return result;
  }

  Rect Rect::Offset(int32_t value) const
  {
    auto result{ *this };

    result.Left -= value;
    result.Top -= value;
    result.Right += value;
    result.Bottom += value;

    return result;
  }

  Rect Rect::Fit(float targetAspectRatio) const
  {
    auto sourceSize = Size();
    auto sourceAspectRatio = sourceSize.AspectRatio();

    Graphics::Size targetSize;
    if (sourceAspectRatio > targetAspectRatio)
    {
      targetSize.Width = sourceSize.Width;
      targetSize.Height = int32_t(sourceSize.Width / targetAspectRatio);
    }
    else
    {
      targetSize.Height = sourceSize.Height;
      targetSize.Width = int32_t(sourceSize.Height * targetAspectRatio);
    }

    auto center = Center();
    return FromCenterSize(center, targetSize);
  }

  Rect Rect::Clamp(Graphics::Size size) const
  {
    return {
      clamp(Left, 0, size.Width),
      clamp(Top, 0, size.Height),
      clamp(Right, 0, size.Width),
      clamp(Bottom, 0, size.Height)
    };
  }

  Rect Rect::PushClamp(Graphics::Size size) const
  {
    auto result{ *this };

    if (result.Left < 0)
    {
      auto offset = -result.Left;
      result.Left += offset;
      result.Right += offset;
    }

    if (result.Top < 0)
    {
      auto offset = -result.Top;
      result.Top += offset;
      result.Bottom += offset;
    }

    if (result.Right > size.Width)
    {
      auto offset = result.Right - size.Width;
      result.Left -= offset;
      result.Right -= offset;
    }

    if (result.Bottom > size.Height)
    {
      auto offset = result.Bottom - size.Height;
      result.Top -= offset;
      result.Bottom -= offset;
    }

    return result.Clamp(size);
  }

  bool Rect::Contains(Rect rect) const
  {
    return 
      rect.Left >= Left &&
      rect.Top >= Top &&
      rect.Right <= Right &&
      rect.Bottom <= Bottom;
  }

  Point Rect::LeftTop() const
  {
    return { Left, Top };
  }

  Point Rect::Center() const
  {
    return { (Left + Right) / 2, (Top + Bottom) / 2 };
  }

  Size Rect::Size() const
  {
    return { Right - Left, Bottom - Top };
  }

  Rect::operator bool() const
  {
    return Left < Right && Top < Bottom;
  }
}