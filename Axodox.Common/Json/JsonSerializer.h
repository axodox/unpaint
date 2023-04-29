#pragma once
#include "JsonObject.h"
#include "JsonString.h"
#include "JsonBoolean.h"
#include "JsonNumber.h"
#include "JsonArray.h"
#include "JsonNull.h"

namespace Axodox::Json
{
  class json_property_base
  {
  public:
    json_property_base(const char* name);

    const char* name() const;

    virtual std::unique_ptr<json_value> to_json() const = 0;
    virtual bool from_json(const json_value* json) = 0;

  private:
    const char* _name;
  };

  class json_object_base
  {
    template<typename value_t> friend class json_property;
    template<typename value_t, typename enable_t> friend struct json_serializer;

  private:
    std::vector<ptrdiff_t> _propertyOffsets;
  };

  template<typename value_t>
  class json_property : json_property_base
  {
  public:
    json_property(json_object_base* owner, const char* name, value_t value = value_t{}) :
      json_property_base(name),
      _value(value)
    {
      owner->_propertyOffsets.push_back(ptrdiff_t((uint8_t*)this - (uint8_t*)owner));
    }

    value_t& operator*()
    {
      return _value;
    }

    const value_t& operator*() const
    {
      return _value;
    }

    virtual std::unique_ptr<json_value> to_json() const override
    {
      return json_serializer<value_t>::to_json(_value);
    }

    virtual bool from_json(const json_value* json) override
    {
      return json_serializer<value_t>::from_json(json, _value);
    }

  private:
    value_t _value;
  };

  template <typename value_t>
  struct json_serializer<value_t, std::enable_if_t<std::is_base_of_v<json_object_base, value_t>, void>>
  {
    static std::unique_ptr<json_value> to_json(const value_t& value)
    {
      auto result = std::make_unique<json_object>();

      for (auto propertyOffset : value._propertyOffsets)
      {
        auto property = (const json_property_base*)((const uint8_t*)&value + propertyOffset);
        result->set_value(property->name(), property->to_json());
      }

      return result;
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      if (!json || json->type() != json_type::object) return false;

      auto jsonObject = static_cast<const json_object*>(json);

      for (auto propertyOffset : value._propertyOffsets)
      {
        auto property = (json_property_base*)((uint8_t*)&value + propertyOffset);

        json_value* propertyValue;
        if (jsonObject->try_get_value(property->name(), propertyValue))
        {
          property->from_json(propertyValue);
        }
      }

      return true;
    }
  };
}