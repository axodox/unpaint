#pragma once
#include "Tensor.h"
#include "OnnxEnvironment.h"
#include "LMSDiscreteScheduler.h"

namespace Axodox::MachineLearning
{
  struct StableDiffusionOptions
  {
    size_t StepCount = 15;    
    size_t BatchSize = 1;
    size_t Width = 512;
    size_t Height = 512;
    float GuidanceScale = 7.f;    
    uint32_t Seed = 0;
    Tensor TextEmbeddings;
  };

  class StableDiffusionInferer
  {
    struct StableDiffusionContext
    {
      StableDiffusionOptions Options;
      LmsDiscreteScheduler Scheduler;
      std::minstd_rand Random;
    };

  public:
    StableDiffusionInferer(OnnxEnvironment& environment);

    Tensor RunInference(const StableDiffusionOptions& options = {});

  private:
    OnnxEnvironment& _environment;
    Ort::Session _session;

    std::uniform_real_distribution<float> _floatDistribution;

    Tensor GenerateLatentSample(StableDiffusionContext& context);
  };
}