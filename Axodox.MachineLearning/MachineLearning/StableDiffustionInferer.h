#pragma once
#include "Tensor.h"
#include "OnnxEnvironment.h"
#include "StableDiffusionScheduler.h"

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
      StableDiffusionScheduler Scheduler;
      std::vector<std::minstd_rand> Randoms;
    };

  public:
    StableDiffusionInferer(OnnxEnvironment& environment);

    Tensor RunInference(const StableDiffusionOptions& options = {});

  private:
    OnnxEnvironment& _environment;
    Ort::Session _session;

    Tensor GenerateLatentSample(StableDiffusionContext& context);
  };
}