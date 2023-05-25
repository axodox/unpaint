#include "pch.h"
#include "Half.h"

using namespace DirectX::PackedVector;

namespace Axodox::Infrastructure
{
  void half::operator=(float value)
  {
    _value = XMConvertFloatToHalf(value);
  }

  half::operator float() const
  {
    return XMConvertHalfToFloat(_value);
  }
}