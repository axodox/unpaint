#include "pch.h"
#include "HuggingFaceSchema.h"

namespace Axodox::Web
{
  HuggingFaceModelInfo::HuggingFaceModelInfo() :
    Id(this, "id")
  { }

  HuggingFaceFileRef::HuggingFaceFileRef() :
    FilePath(this, "rfilename")
  { }

  HuggingFaceModelDetails::HuggingFaceModelDetails() :
    Id(this, "id"),
    Author(this, "author"),
    Downloads(this, "downloads"),
    Likes(this, "likes"),
    Tags(this, "tags"),
    Files(this, "siblings")
  { }
  
  const std::set<std::string> HuggingFaceModelDetails::StableDiffusionOnnxFileset = {
    "tokenizer/vocab.json",
    "tokenizer/merges.txt",
    "tokenizer/special_tokens_map.json",
    "tokenizer/tokenizer_config.json",
    "unet/model.onnx",
    "vae_decoder/model.onnx",
    "vae_encoder/model.onnx",
    "scheduler/scheduler_config.json",
    "text_encoder/model.onnx"
  };

  bool HuggingFaceModelDetails::IsValidModel(const std::set<std::string>& fileset)
  {
    auto count = 0;
    for (auto& file : *Files)
    {
      if (fileset.contains(*file.FilePath)) count++;
    }

    return count == fileset.size();
  }
}