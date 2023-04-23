#include "pch.h"
#include "StableDiffustionInferer.h"
#include "VaeDecoder.h"
#include "OnnxModelStatistics.h"

using namespace DirectX;
using namespace Ort;
using namespace std;

namespace Axodox::MachineLearning
{
  StableDiffusionInferer::StableDiffusionInferer(OnnxEnvironment& environment) :
    _environment(environment),
    _session(nullptr)
  {
    _session = { _environment.Environment(), (_environment.RootPath() / L"unet/model.onnx").c_str(), _environment.DefaultSessionOptions() };
    OnnxPrintStatistics(_environment, _session);
  }

  Tensor StableDiffusionInferer::RunInference(const StableDiffusionOptions& options)
  {
    if (options.LatentInput && options.LatentInput.Shape[0] != options.BatchSize) throw invalid_argument("options.BatchSize");

    StableDiffusionContext context{
      .Options = options
    };

    context.Randoms.reserve(options.BatchSize);
    for (size_t i = 0; i < options.BatchSize; i++)
    {
      context.Randoms.push_back(minstd_rand{ options.Seed + uint32_t(i) });
    }

    list<Tensor> derivatives;

    auto initialStep = clamp(options.StepCount - size_t(options.StepCount * options.DenoisingStrength) - 1, size_t(0), options.StepCount);
    auto latentSample = options.LatentInput ? PrepareLatentSample(context, options.LatentInput, initialStep) : GenerateLatentSample(context);

    IoBinding binding{ _session };
    binding.BindOutput("out_sample", _environment.MemoryInfo());
    binding.BindInput("encoder_hidden_states", options.TextEmbeddings.ToOrtValue(_environment.MemoryInfo()));
        
    auto steps = context.Scheduler.GetSteps(options.StepCount);
    for (size_t i = initialStep; i < steps.Timesteps.size(); i++)
    {
      printf("Step %zd\n", i + 1);
      auto scaledSample = latentSample.Duplicate().Swizzle(options.BatchSize) / sqrt(steps.Sigmas[i] * steps.Sigmas[i] + 1);

      binding.BindInput("sample", scaledSample.ToOrtValue(_environment.MemoryInfo()));
      binding.BindInput("timestep", Tensor(steps.Timesteps[i]).ToOrtValue(_environment.MemoryInfo()));

      _session.Run({}, binding);

      auto outputs = binding.GetOutputValues();
      auto output = Tensor::FromOrtValue(outputs[0]);

      auto outputComponents = output.Swizzle().Split();

      auto& blankNoise = outputComponents[0];
      auto& textNoise = outputComponents[1];
      auto guidedNoise = blankNoise.BinaryOperation<float>(textNoise, [guidanceScale = options.GuidanceScale](float a, float b) 
        { return a + guidanceScale * (b - a); });

      latentSample = steps.ApplyStep(latentSample, guidedNoise, derivatives, context.Randoms, i);
    }

    latentSample = latentSample * (1.0f / 0.18215f);
    return latentSample;
  }

  Tensor StableDiffusionInferer::PrepareLatentSample(StableDiffusionContext& context, const Tensor& latents, size_t initialStep) const
  {
    auto cumulativeAlpha = context.Scheduler.CumulativeAlphas()[initialStep];
    auto inputFactor = sqrt(cumulativeAlpha) * 0.18215f;
    auto noiseFactor = sqrt(1 - cumulativeAlpha);

    auto result = Tensor::CreateRandom(latents.Shape, context.Randoms);
    result.UnaryOperation<float>(latents, [=](float a, float b) { return a * noiseFactor + b * inputFactor; });
    return result;
  }
  
  Tensor StableDiffusionInferer::GenerateLatentSample(StableDiffusionContext& context) const
  {
    Tensor::shape_t shape{ context.Options.BatchSize, 4, context.Options.Height / 8, context.Options.Width / 8 };
    return Tensor::CreateRandom(shape, context.Randoms, context.Scheduler.InitialNoiseSigma());
  }
}