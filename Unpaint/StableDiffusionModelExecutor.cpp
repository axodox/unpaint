#include "pch.h"
#include "StableDiffusionModelExecutor.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "MachineLearning/TextEmbedder.h"
#include "MachineLearning/VaeEncoder.h"
#include "MachineLearning/VaeDecoder.h"
#include "Threading/Parallel.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace std;

namespace winrt::Unpaint
{
  const char* const StableDiffusionModelExecutor::_safetyFilter = "nsfw, nudity, porn, sex, child, girl, boy, minor, teen, ";

  StableDiffusionModelExecutor::StableDiffusionModelExecutor() :
    _unpaintOptions(dependencies.resolve<UnpaintOptions>()),
    _modelRepository(dependencies.resolve<ModelRepository>())
  { }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation)
  {
    //Set up async source
    lock_guard lock(_mutex);
    thread_name_context threadName{L"* inference"};

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

    if (!task.InputImage.empty())
    {
      auto imageTensor = LoadImage(task, async);
      inputs.InputImage = EncodeVAE(imageTensor, async);

      inputs.InputMask = Tensor{ TensorType::Single, 1, 1, inputs.InputImage.Shape[2], inputs.InputImage.Shape[3] };
      inputs.InputMask.Fill(1.f);
    }

    //Run diffusion
    auto latentImage = RunStableDiffusion(task, inputs, async);
    if (async.is_cancelled()) return {};

    //Prepare outputs
    auto decocedImage = DecodeVAE(latentImage, async);

    //Return results
    async.update_state(1.f, "Done.");
    return decocedImage;
  }
  
  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::LoadImage(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading input image...");
    auto imageBuffer = try_read_file(task.InputImage);
    auto imageTexture = TextureData::FromBuffer(imageBuffer);
    imageTexture = imageTexture.Resize(task.Resolution.x, task.Resolution.y);
    return Tensor::FromTextureData(imageTexture);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE encoder...");
    VaeEncoder vaeEncoder{ *_onnxEnvironment };

    async.update_state("Encoding color image...");
    return vaeEncoder.EncodeVae(colorImage);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    if (_textEmbedding && _positivePrompt == task.PositivePrompt && _negativePrompt == task.NegativePrompt) return _textEmbedding;

    async.update_state(NAN, "Loading text embedder...");
    TextEmbedder textEmbedder{ *_onnxEnvironment, app_folder() };

    async.update_state("Creating text embedding...");
    auto encodedNegativePrompt = textEmbedder.ProcessText((task.SafeMode ? _safetyFilter : "") + task.NegativePrompt);
    auto encodedPositivePrompt = textEmbedder.ProcessText(task.PositivePrompt);

    _positivePrompt = task.PositivePrompt;
    _negativePrompt = task.NegativePrompt;
    _textEmbedding = encodedNegativePrompt.Concat(encodedPositivePrompt);
    
    async.update_state("Text embedding created.");

    return _textEmbedding;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading denoiser...");
    if (!_denoiser) _denoiser = make_unique<StableDiffusionInferer>(*_onnxEnvironment);

    StableDiffusionOptions options{
      .StepCount = task.SamplingSteps,
      .BatchSize = 1,
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