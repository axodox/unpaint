#include "Json/JsonSerializer.h"

namespace Axodox::Web
{
  struct HuggingFaceModelInfo : public Json::json_object_base
  {
    Json::json_property<std::string> Id;

    HuggingFaceModelInfo();
  };

  struct HuggingFaceFileRef : public Json::json_object_base
  {
    Json::json_property<std::string> FilePath;

    HuggingFaceFileRef();
  };

  struct HuggingFaceModelDetails : public Json::json_object_base
  {
    Json::json_property<std::string> Id;
    Json::json_property<std::string> Author;
    Json::json_property<uint32_t> Downloads;
    Json::json_property<uint32_t> Likes;
    Json::json_property<std::vector<std::string>> Tags;
    Json::json_property<std::vector<HuggingFaceFileRef>> Files;

    HuggingFaceModelDetails();
  };
}