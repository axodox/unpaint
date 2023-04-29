#include "pch.h"
#include "JsonSerializer.h"

namespace Axodox::Json
{
  json_property_base::json_property_base(const char* name) :
    _name(name)
  { }

  const char* json_property_base::name() const
  {
    return _name;
  }
}