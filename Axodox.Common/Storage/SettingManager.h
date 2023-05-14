#pragma once
#include "pch.h"
#include "MemoryStream.h"
#include "ArrayStream.h"

namespace Axodox::Storage
{
  class SettingManager
  {
  public:
    SettingManager();

    bool HasSetting(std::string_view key);
    void RemoveSetting(std::string_view key);

    template <typename T>
    void StoreSetting(std::string_view key, const T& value)
    {
      _settings.Insert(winrt::to_hstring(key), SerializeValue(value));
    }

    template <typename T>
    bool TryLoadSetting(std::string_view key, T& value)
    {
      if (HasSetting(key))
      {
        auto setting = _settings.Lookup(winrt::to_hstring(key));
        auto result = TryDeserializeValue<T>(setting);
        if (result)
        {
          value = *result;
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    template <typename T>
    T LoadSettingOr(std::string_view key, const T& fallback = T{})
    {
      T result = fallback;
      TryLoadSetting(key, result);
      return result;
    }

  private:
    winrt::Windows::Foundation::Collections::IPropertySet _settings;

    template <typename T>
    winrt::Windows::Foundation::IInspectable SerializeValue(const T& value)
    {
      memory_stream stream;
      stream.write(value);

      return winrt::Windows::Foundation::PropertyValue::CreateUInt8Array(winrt::array_view<uint8_t>{stream.data(), uint32_t(stream.length())});
    }

    template <typename T>
    std::optional<T> TryDeserializeValue(winrt::Windows::Foundation::IInspectable const& value)
    {
      try
      {
        auto propertyValue = value.as<winrt::Windows::Foundation::IPropertyValue>();

        winrt::com_array<uint8_t> rawValue;
        propertyValue.GetUInt8Array(rawValue);

        T result;
        array_stream stream{ std::span<const uint8_t>{rawValue} };
        stream.read(result);

        return result;
      }
      catch (...)
      {
        return std::nullopt;
      }
    }
  };
}