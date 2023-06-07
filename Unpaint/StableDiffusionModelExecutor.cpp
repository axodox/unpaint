#include "pch.h"
#include "StableDiffusionModelExecutor.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "MachineLearning/VaeEncoder.h"
#include "MachineLearning/VaeDecoder.h"
#include "MachineLearning/SafetyChecker.h"
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
    _stepCount(0),
    _isSafeModeEnabled(true)
  { }

  int32_t StableDiffusionModelExecutor::ValidatePrompt(std::string_view modelId, std::string prompt, bool isSafeModeEnabled)
  {
    lock_guard lock(_mutex);
    EnsureEnvironment(modelId);
    if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_onnxEnvironment, app_folder());

    if (isSafeModeEnabled) prompt = _safetyFilter + prompt;

    return _textEmbedder->ValidatePrompt(prompt);
  }

  std::vector<Axodox::Graphics::TextureData> StableDiffusionModelExecutor::TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation)
  {
    //Set up async source
    lock_guard lock(_mutex);
    thread_name_context threadName{ L"* inference" };

    async_operation_source async;
    operation.set_source(async);

    try
    {
      //Initialize execution environment
      async.update_state(NAN, "Initializing execution environment...");
      EnsureEnvironment(task.ModelId);

      //Prepare inputs
      StableDiffusionInputs inputs;
      inputs.TextEmbeddings = CreateTextEmbeddings(task, async);

      TextureData targetTexture;
      auto sourceRect = Rect::Empty;
      auto targetRect = Rect::Empty;
      if (!task.InputImage.empty())
      {
        inputs.InputMask = LoadMask(task, sourceRect, targetRect, async);

        auto imageTensor = LoadImage(task, targetTexture, sourceRect, targetRect, async);
        inputs.InputImage = EncodeVAE(imageTensor, async);        
      }

      //Run diffusion
      auto latentImage = RunStableDiffusion(task, inputs, async);
      if (async.is_cancelled()) return {};

      //Prepare outputs
      auto decodedImage = DecodeVAE(latentImage, async);
      auto outputs = decodedImage.ToTextureData();
      if (task.InputMask && (sourceRect || targetRect))
      {
        if (!targetRect) targetRect = Rect::FromSize(targetTexture.Size());
                
        for (auto& output : outputs)
        {
          if (!sourceRect) sourceRect = Rect::FromSize(output.Size());

          auto targetSize = targetRect.Size();
          output = targetTexture.MergeTexture(output.GetTexture(sourceRect).Resize(uint32_t(targetSize.Width), uint32_t(targetSize.Height)), targetRect.LeftTop());

          output = TextureData::AlphaBlend(targetTexture, output, task.InputMask);
        }
      }

      //Safety check
      if (task.IsSafetyCheckerEnabled) RunSafetyCheck(outputs, async);

      //Return results
      async.update_state(1.f, "Done.");
      return outputs;
    }
    catch (...)
    {
      async.update_state(1.f, "Generation failed.");
      return {};
    }
  }

  void StableDiffusionModelExecutor::EnsureEnvironment(std::string_view modelId)
  {
    if (_modelId != modelId || !_onnxEnvironment)
    {
      _textEmbedder.reset();
      _denoiser.reset();

      _onnxEnvironment = make_unique<OnnxEnvironment>(_modelRepository->Root() / modelId);      
      _modelId = modelId;

      _stepCount = 0;
      _positivePrompt.clear();
      _negativePrompt.clear();
      _textEmbedding.clear();

      _inputImage = {};
      _inputLatent = {};
    }
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::LoadImage(const StableDiffusionInferenceTask& task, Axodox::Graphics::TextureData& sourceTexture, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading input image...");
    auto imageBuffer = try_read_file(task.InputImage);

    sourceTexture = TextureData::FromBuffer(imageBuffer);
    auto imageTexture{ sourceTexture };

    if (targetRect)
    {
      imageTexture = imageTexture.GetTexture(targetRect);
    }
    
    if (imageTexture.Width != task.Resolution.x || imageTexture.Height != task.Resolution.y)
    {
      imageTexture = imageTexture.UniformResize(task.Resolution.x, task.Resolution.y, &sourceRect);
    }

    return Tensor::FromTextureData(imageTexture);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::LoadMask(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async)
  {
    if (!task.InputMask) return {};

    async.update_state(NAN, "Loading input mask...");

    //Calculate mask size and return nothing if empty
    auto maskedRect = task.InputMask.FindNonZeroRect();
    if (!maskedRect) return {};

    Size targetSize{ int32_t(task.Resolution.x), int32_t(task.Resolution.y) };
    targetRect = maskedRect
      .Offset(16)
      .Fit(targetSize.AspectRatio())
      .PushClamp(task.InputMask.Size());

    auto maskTexture = task.InputMask.GetTexture(targetRect);

    //Fit mask to image
    if (maskTexture.Width != task.Resolution.x || maskTexture.Height != task.Resolution.y)
    {
      maskTexture = maskTexture.UniformResize(task.Resolution.x, task.Resolution.y, &sourceRect);
    }

    maskTexture = maskTexture.Resize(maskTexture.Width / 8, maskTexture.Height / 8);

    return Tensor::FromTextureData(maskTexture);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async)
  {
    if (colorImage == _inputImage && _inputLatent) return _inputLatent;

    async.update_state(NAN, "Loading VAE encoder...");
    VaeEncoder vaeEncoder{ *_onnxEnvironment };

    async.update_state("Encoding color image...");
    _inputLatent = vaeEncoder.EncodeVae(colorImage);
    _inputImage = colorImage;

    return _inputLatent;
  }

  Axodox::MachineLearning::ScheduledTensor StableDiffusionModelExecutor::CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    //Check if the prompt has changed
    if (!_textEmbedding.empty() && _positivePrompt == task.PositivePrompt && _negativePrompt == task.NegativePrompt && task.SamplingSteps == _stepCount && _isSafeModeEnabled == task.IsSafeModeEnabled) return _textEmbedding;

    //Load embedder
    async.update_state(NAN, "Loading text embedder...");
    if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_onnxEnvironment, app_folder());

    //Parse and schedule prompt
    async.update_state("Creating text embedding...");
    auto encodedNegativePrompt = _textEmbedder->SchedulePrompt((task.IsSafeModeEnabled ? _safetyFilter : "") + task.NegativePrompt, task.SamplingSteps);
    auto encodedPositivePrompt = _textEmbedder->SchedulePrompt(task.PositivePrompt, task.SamplingSteps);

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
    _isSafeModeEnabled = task.IsSafeModeEnabled;

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

    /*if (!_unpaintOptions->IsDenoiserPinned())*/ _denoiser.reset();

    return result;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE decoder...");
    VaeDecoder vaeDecoder{ *_onnxEnvironment };

    async.update_state("Decoding latent image...");
    return vaeDecoder.DecodeVae(latentImage);
  }

  void StableDiffusionModelExecutor::RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& images, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading safety checker...");
    SafetyChecker safetyChecker{ *_onnxEnvironment };

    async.update_state(NAN, "Checking safety...");
    for (auto& image : images)
    {
      if (!safetyChecker.IsSafe(image))
      {
        async.update_state(NAN, "Unsafe image encountered.");
        image = {};
      }
    }
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
    *result.SafeMode = IsSafeModeEnabled;

    *result.ModelId = ModelId;

    return result;
  }
}