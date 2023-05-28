#include "pch.h"
#include "StableDiffusionModelExecutor.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "MachineLearning/TextEmbedder.h"
#include "MachineLearning/VaeEncoder.h"
#include "MachineLearning/VaeDecoder.h"
#include "Threading/Parallel.h"
#include "Collections/Hasher.h"

using namespace Axodox::Collections;
using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;

namespace winrt::Unpaint
{
  const char* const StableDiffusionModelExecutor::_safetyFilter = "nsfw, nudity, porn, sex, child, girl, boy, minor, teen, ";

  StableDiffusionModelExecutor::StableDiffusionModelExecutor() :
    _unpaintOptions(dependencies.resolve<UnpaintOptions>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _stepCount(0)
  { }

  std::vector<Axodox::Graphics::TextureData> StableDiffusionModelExecutor::TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation)
  {
    //Set up async source
    lock_guard lock(_mutex);
    thread_name_context threadName{ L"* inference" };

    async_operation_source async;
    operation.set_source(async);

    //Initialize execution environment
    if (_modelId != task.ModelId || !_onnxEnvironment)
    {
      async.update_state(NAN, "Initializing execution environment...");

      _onnxEnvironment = make_unique<OnnxEnvironment>(_modelRepository->Root() / task.ModelId);
      _modelId = task.ModelId;
    }

    //Prepare inputs
    StableDiffusionInputs inputs;
    inputs.TextEmbeddings = CreateTextEmbeddings(task, async);

    std::optional<XMUINT2> resolutionOverride;
    if (!task.InputImage.empty())
    {
      auto imageTensor = LoadImage(task, resolutionOverride, async);
      inputs.InputImage = EncodeVAE(imageTensor, async);

      /*inputs.InputMask = Tensor{ TensorType::Single, 1, 1, inputs.InputImage.Shape[2], inputs.InputImage.Shape[3] };
      inputs.InputMask.Fill(1.f);*/
    }

    //Run diffusion
    auto latentImage = RunStableDiffusion(task, inputs, async);
    if (async.is_cancelled()) return {};

    //Prepare outputs
    auto decodedImage = DecodeVAE(latentImage, async);
    auto outputs = decodedImage.ToTextureData();
    if (resolutionOverride)
    {
      for (auto& output : outputs)
      {
        if (output.Width > resolutionOverride->x)
        {
          output = output.TruncateHorizontally(resolutionOverride->x);
        }

        if (output.Height > resolutionOverride->y)
        {
          output = output.TruncateVertically(resolutionOverride->y);
        }
      }
    }

    //Return results
    async.update_state(1.f, "Done.");
    return outputs;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::LoadImage(const StableDiffusionInferenceTask& task, std::optional<DirectX::XMUINT2>& resolutionOverride, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading input image...");
    auto imageBuffer = try_read_file(task.InputImage);
    auto imageTexture = TextureData::FromBuffer(imageBuffer);

    auto sourceAspectRatio = float(imageTexture.Width) / float(imageTexture.Height);
    auto targetAspectRatio = float(task.Resolution.x) / float(task.Resolution.y);

    uint32_t width, height;
    if (targetAspectRatio > sourceAspectRatio)
    {
      width = uint32_t(task.Resolution.x * sourceAspectRatio / targetAspectRatio);
      height = task.Resolution.y;
    }
    else
    {
      width = task.Resolution.x;
      height = uint32_t(task.Resolution.y * targetAspectRatio / sourceAspectRatio);
    }

    imageTexture = imageTexture.Resize(width, height);

    if (!are_equal(task.Resolution, XMUINT2{ width, height }))
    {
      if (targetAspectRatio > sourceAspectRatio)
      {
        imageTexture = imageTexture.ExtendHorizontally(task.Resolution.x);
      }
      else
      {
        imageTexture = imageTexture.ExtendVertically(task.Resolution.y);
      }

      resolutionOverride = { width, height };
    }

    return Tensor::FromTextureData(imageTexture);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE encoder...");
    VaeEncoder vaeEncoder{ *_onnxEnvironment };

    async.update_state("Encoding color image...");
    return vaeEncoder.EncodeVae(colorImage);
  }

  Axodox::MachineLearning::ScheduledTensor StableDiffusionModelExecutor::CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    //Check if the prompt has changed
    if (!_textEmbedding.empty() && _positivePrompt == task.PositivePrompt && _negativePrompt == task.NegativePrompt && task.SamplingSteps == _stepCount) return _textEmbedding;

    //Load embedder
    async.update_state(NAN, "Loading text embedder...");
    TextEmbedder textEmbedder{ *_onnxEnvironment, app_folder() };

    //Parse and schedule prompt
    async.update_state("Creating text embedding...");
    auto encodedNegativePrompt = textEmbedder.ScheduleText((task.SafeMode ? _safetyFilter : "") + task.NegativePrompt, task.SamplingSteps);
    auto encodedPositivePrompt = textEmbedder.ScheduleText(task.PositivePrompt, task.SamplingSteps);

    //Concatenate negative and position prompts
    _textEmbedding.resize(task.SamplingSteps);
    trivial_map<pair<void*, void*>, shared_ptr<Tensor>> embeddingBuffer;
    for (auto i = 0u; i < task.SamplingSteps; i++)
    {
      auto& concatenatedTensor = embeddingBuffer[{ encodedNegativePrompt[i].get(), encodedPositivePrompt[i].get() }];
      if (!concatenatedTensor)
      {
        concatenatedTensor = make_shared<Tensor>(encodedNegativePrompt[i]->Concat(*encodedPositivePrompt[i]));
      }

      _textEmbedding[i] = concatenatedTensor;
    }

    //Update cache key
    _positivePrompt = task.PositivePrompt;
    _negativePrompt = task.NegativePrompt;
    _stepCount = task.SamplingSteps;

    //Return result
    async.update_state("Text embedding created.");
    return _textEmbedding;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading denoiser...");
    if (!_denoiser) _denoiser = make_unique<StableDiffusionInferer>(*_onnxEnvironment);

    StableDiffusionOptions options{
      .StepCount = task.SamplingSteps,
      .BatchSize = task.BatchSize,
      .Width = task.Resolution.x,
      .Height = task.Resolution.y,
      .GuidanceScale = task.GuidanceStrength,
      .Seed = task.RandomSeed,
      .TextEmbeddings = inputs.TextEmbeddings,
      .LatentInput = inputs.InputImage,
      .MaskInput = inputs.InputMask,
      .DenoisingStrength = task.Mode == InferenceMode::Modify ? task.DenoisingStrength : 1.f
    };

    async.update_state("Running denoiser...");
    auto result = _denoiser->RunInference(options, &async);

    if (!_unpaintOptions->IsDenoiserPinned()) _denoiser.reset();

    return result;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE decoder...");
    VaeDecoder vaeDecoder{ *_onnxEnvironment };

    async.update_state("Decoding latent image...");
    return vaeDecoder.DecodeVae(latentImage);
  }

  ImageMetadata StableDiffusionInferenceTask::ToMetadata() const
  {
    ImageMetadata result{};

    *result.PositivePrompt = PositivePrompt;
    *result.NegativePrompt = NegativePrompt;

    *result.Width = Resolution.x;
    *result.Height = Resolution.y;

    *result.GuidanceStrength = GuidanceStrength;
    *result.DenoisingStrength = DenoisingStrength;
    *result.SamplingSteps = SamplingSteps;
    *result.RandomSeed = RandomSeed;
    *result.SafeMode = SafeMode;

    *result.ModelId = ModelId;

    return result;
  }
}