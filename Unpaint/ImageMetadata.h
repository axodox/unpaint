#pragma once
#include "Json/JsonSerializer.h"

namespace winrt::Unpaint
{
  struct ImageMetadata : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<std::string> PositivePrompt;
    Axodox::Json::json_property<std::string> NegativePrompt;

    Axodox::Json::json_property<uint32_t> Width;
    Axodox::Json::json_property<uint32_t> Height;

    Axodox::Json::json_property<float> GuidanceStrength;
    Axodox::Json::json_property<uint32_t> SamplingSteps;
    Axodox::Json::json_property<uint32_t> RandomSeed;
    Axodox::Json::json_property<bool> SafeMode;

    Axodox::Json::json_property<std::string> ModelId;

    ImageMetadata();
  };
}