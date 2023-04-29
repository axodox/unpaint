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
}