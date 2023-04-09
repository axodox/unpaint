#pragma once
#include "TensorType.h"
#include "Graphics/Textures/TextureData.h"

namespace Axodox::MachineLearning
{
  struct Tensor
  {
    typedef std::array<size_t, 4> shape_t;

    TensorType Type;
    shape_t Shape;
    std::vector<uint8_t> Buffer;

    Tensor();
    Tensor(TensorType type, size_t x = 0, size_t y = 0, size_t z = 0, size_t w = 0);
    Tensor(TensorType type, shape_t shape);

    template<typename T>
    Tensor(T value) :
      Type(ToTensorType<T>()),
      Shape(1, 0, 0, 0)
    {
      AllocateBuffer();
      *AsPointer<T>() = value;
    }

    void AllocateBuffer();

    size_t ByteCount() const;
    bool IsValid() const;
    void ThrowIfInvalid() const;

    size_t Size(size_t dimension = 0) const;

    static Tensor FromOrtValue(const Ort::Value& value);
    Ort::Value ToOrtValue(Ort::MemoryInfo& memoryInfo) const;

    void UpdateOrtValue(Ort::Value& value);

    std::vector<Graphics::TextureData> ToTextureData() const;

    const uint8_t* AsPointer(size_t x = 0, size_t y = 0, size_t z = 0, size_t w = 0) const;
    uint8_t* AsPointer(size_t x = 0, size_t y = 0, size_t z = 0, size_t w = 0);

    template<typename T>
    const T* AsPointer(size_t x = 0, size_t y = 0, size_t z = 0, size_t w = 0) const
    {
      if (ToTensorType<T>() != Type) throw std::bad_cast();
      return reinterpret_cast<const T*>(AsPointer(x, y, z, w));
    }

    template<typename T>
    T* AsPointer(size_t x = 0, size_t y = 0, size_t z = 0, size_t w = 0)
    {
      if (ToTensorType<T>() != Type) throw std::bad_cast();
      return reinterpret_cast<T*>(AsPointer(x, y, z, w));
    }

    template<typename T>
    std::span<const T> AsSpan() const
    {
      if (ToTensorType<T>() != Type) throw std::bad_cast();
      return std::span<const T>(reinterpret_cast<const T*>(Buffer.data()), Size());
    }

    template<typename T>
    std::span<T> AsSpan()
    {
      if (ToTensorType<T>() != Type) throw std::bad_cast();
      return std::span<T>(reinterpret_cast<T*>(Buffer.data()), Size());
    }

    template<typename T>
    Tensor operator*(T value) const
    {
      Tensor result{ *this };
      for (auto& item : result.AsSpan<T>())
      {
        item *= value;
      }
      return result;
    }

    template<typename T>
    Tensor operator/(T value) const
    {
      return *this * (T(1) / value);
    }

    Tensor Duplicate(size_t instances = 2) const;

    std::vector<Tensor> Split(size_t instances = 2) const;

    template<typename T>
    Tensor BinaryOperation(const Tensor& other, const std::function<T(T, T)>& operation) const
    {
      if (Shape != other.Shape) throw std::logic_error("Incompatible tensor shapes.");
      if (Type != other.Type) throw std::logic_error("Incompatible tensor types.");

      Tensor result{ Type, Shape };

      auto size = Size();
      auto a = AsPointer<T>();
      auto b = other.AsPointer<T>();
      auto c = result.AsPointer<T>();
      for (size_t i = 0; i < size; i++)
      {
        *c++ = operation(*a++, *b++);
      }

      return result;
    }

    template<typename T>
    void UnaryOperation(const Tensor& other, const std::function<T(T, T)>& operation)
    {
      if (Shape != other.Shape) throw std::logic_error("Incompatible tensor shapes.");
      if (Type != other.Type) throw std::logic_error("Incompatible tensor types.");
      
      auto size = Size();
      auto a = AsPointer<T>();
      auto b = other.AsPointer<T>();
      for (size_t i = 0; i < size; i++)
      {
        *a++ = operation(*a, *b++);
      }
    }

    static std::pair<TensorType, Tensor::shape_t> ToTypeAndShape(const Ort::TensorTypeAndShapeInfo& info);
  };
}