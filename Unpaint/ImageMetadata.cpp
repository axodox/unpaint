#include "pch.h"
#include "ImageMetadata.h"

namespace winrt::Unpaint
{
  ImageMetadata::ImageMetadata() :
    PositivePrompt(this, "PositivePrompt"),
    NegativePrompt(this, "NegativePrompt"),
    Width(this, "Width"),
    Height(this, "Height"),
    GuidanceStrength(this, "GuidanceStrength"),
    DenoisingStrength(this, "DenoisingStrength", 1.f),
    SamplingSteps(this, "SamplingSteps"),
    RandomSeed(this, "RandomSeed"),
    SafeMode(this, "SafeMode"),
    ModelId(this, "ModelId")
  { }
}