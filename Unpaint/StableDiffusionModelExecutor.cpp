#include "pch.h"
#include "StableDiffusionModelExecutor.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "MachineLearning/TextTokenizer.h"
#include "MachineLearning/TextEncoder.h"
#include "MachineLearning/VaeDecoder.h"
#include "Threading/Parallel.h"

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
    auto textEmbeddings = CreateTextEmbeddings(task, async);
    auto latentImage = RunStableDiffusion(task, textEmbeddings, async);
    if (async.is_cancelled()) return {};

    auto decocedImage = DecodeVAE(latentImage, async);

    //Return results
    async.update_state(1.f, "Done.");
    return decocedImage;
  }
  
  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    if (_textEmbedding && _positivePrompt == task.PositivePrompt && _negativePrompt == task.NegativePrompt) return _textEmbedding;

    async.update_state(NAN, "Loading text tokenizer...");
    TextTokenizer textTokenizer{ *_onnxEnvironment, app_folder() };

    async.update_state("Loading text encoder...");
    TextEncoder textEncoder{ *_onnxEnvironment };

    async.update_state("Creating text embedding...");
    auto tokenizedNegativePrompt = textTokenizer.TokenizeText((task.SafeMode ? _safetyFilter : "") + task.NegativePrompt);
    auto encodedNegativePrompt = textEncoder.EncodeText(tokenizedNegativePrompt);

    auto tokenizedPositivePrompt = textTokenizer.TokenizeText(task.PositivePrompt);
    auto encodedPositivePrompt = textEncoder.EncodeText(tokenizedPositivePrompt);

    async.update_state("Text embedding created.");

    _positivePrompt = task.PositivePrompt;
    _negativePrompt = task.NegativePrompt;
    _textEmbedding = encodedNegativePrompt.Concat(encodedPositivePrompt);

    return _textEmbedding;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::RunStableDiffusion(const StableDiffusionInferenceTask& task, const Axodox::MachineLearning::Tensor& textEmbeddings, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading denoiser...");
    if (!_denoiser) _denoiser = make_unique<StableDiffusionInferer>(*_onnxEnvironment);

    StableDiffusionOptions options{
      .StepCount = task.SamplingSteps,
      .BatchSize = 1,
      .Width = task.Resolution.x,
      .Height = task.Resolution.y,
      .Seed = task.RandomSeed,
      .TextEmbeddings = textEmbeddings
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
    *result.SamplingSteps = SamplingSteps;
    *result.RandomSeed = RandomSeed;
    *result.SafeMode = SafeMode;

    *result.ModelId = ModelId;

    return result;
  }
}