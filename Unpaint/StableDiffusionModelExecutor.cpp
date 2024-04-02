#include "pch.h"
#include "StableDiffusionModelExecutor.h"

using namespace Axodox::Collections;
using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::MachineLearning;
using namespace Axodox::MachineLearning::Imaging::StableDiffusion;
using namespace Axodox::MachineLearning::Sessions;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace DirectX;
using namespace std;

namespace winrt::Unpaint
{
  const char* StableDiffusionInferenceTask::PositivePromptPlaceholder = "an empty canvas standing in a painter's workshop";
  const char* StableDiffusionInferenceTask::NegativePromptPlaceholder = "blurry, render";

  const char* const StableDiffusionModelExecutor::_safetyFilter = "nsfw, nudity, porn, sex, child, girl, boy, minor, teen, ";

  StableDiffusionModelExecutor::StableDiffusionModelExecutor() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _controlnetRepository(dependencies.resolve<ControlNetRepository>()),
    _onnxhost(dependencies.resolve<OnnxHost>()),
    _stepCount(0),
    _isSafeModeEnabled(true)
  { }

  int32_t StableDiffusionModelExecutor::ValidatePrompt(std::string_view modelId, std::string prompt, bool isSafeModeEnabled)
  {
    try
    {
      thread_name_context threadName{ "* validate prompt" };

      lock_guard lock(_mutex);
      EnsureEnvironment(modelId);
      if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_sessionParameters);

      if (isSafeModeEnabled) prompt = _safetyFilter + prompt;

      return _textEmbedder->ValidatePrompt(prompt);
    }
    catch (...)
    {
      return -1;
    }
  }

  std::vector<Axodox::Graphics::TextureData> StableDiffusionModelExecutor::TryRunInference(const StableDiffusionInferenceTask& rawTask, Axodox::Threading::async_operation& operation)
  {
    //Set up async source
    thread_name_context threadName{ "* inference" };

    lock_guard lock(_mutex);
    async_operation_source async;
    operation.set_source(async);

    auto targetResolution = Size::Zero;
    if (rawTask.InputImage) targetResolution = rawTask.InputImage.Size();
    else if (rawTask.InputCondition) targetResolution = rawTask.InputCondition.Size();
    else if (rawTask.InputMask) targetResolution = rawTask.InputMask.Size();
    else targetResolution = { int(rawTask.Resolution.x), int(rawTask.Resolution.y) };

    auto task = rawTask;
    if (task.InputImage) task.InputImage = task.InputImage.Resize(targetResolution.Width, targetResolution.Height);
    if (task.InputMask) task.InputMask = task.InputMask.Resize(targetResolution.Width, targetResolution.Height);
    if (task.InputCondition) task.InputCondition = task.InputCondition.Resize(targetResolution.Width, targetResolution.Height);

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
      if (task.InputImage || task.InputCondition)
      {
        inputs.InputMask = LoadMask(task, sourceRect, targetRect, async);

        auto [imageTexture, conditionTexture] = LoadImage(task, sourceRect, targetRect, async);
        if (task.InputImage && (task.DenoisingStrength < 1.f || task.InputMask))
        {
          targetTexture = TextureData{ task.InputImage };
          inputs.InputImage = EncodeVAE(Tensor::FromTextureData(imageTexture, ColorNormalization::LinearPlusMinusOne), async);
        }

        if (task.InputCondition)
        {
          inputs.ConditionImage = Tensor::FromTextureData(conditionTexture, ColorNormalization::LinearZeroToOne);

          if (task.ControlNetMode == "inpaint" && task.InputMask)
          {
            CreateControlNetInpaintCondition(inputs.ConditionImage, task.InputMask, targetRect);
          }
        }
      }

      //Run diffusion
      auto latentImage = RunStableDiffusion(task, inputs, async);
      if (async.is_cancelled()) return {};

      //Prepare outputs
      auto decodedImage = DecodeVAE(latentImage, async);
      auto outputs = decodedImage.ToTextureData(ColorNormalization::LinearPlusMinusOne);
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
      else if (sourceRect)
      {
        for (auto& output : outputs)
        {
          output = output.GetTexture(sourceRect);
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
    if (_modelId == modelId) return;

    _textEmbedder.reset();
    _denoiser.reset();

    _modelId = modelId;
    _sessionParameters = make_unique<StableDiffusionStorageFileMapSessionParameters>(_onnxhost, _modelRepository->GetModelFiles(modelId));

    _stepCount = 0;
    _positivePrompt.clear();
    _negativePrompt.clear();
    _textEmbedding.reset();

    _inputImage = {};
    _inputLatent = {};
  }

  std::pair<Axodox::Graphics::TextureData, Axodox::Graphics::TextureData> StableDiffusionModelExecutor::LoadImage(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading input image...");
    auto imageTexture{ task.InputImage };
    auto conditionTexture{ task.InputCondition };

    if (targetRect)
    {
      imageTexture = imageTexture.GetTexture(targetRect);
      conditionTexture = conditionTexture.GetTexture(targetRect);
    }

    if (imageTexture.Width != task.Resolution.x || imageTexture.Height != task.Resolution.y)
    {
      imageTexture = imageTexture.UniformResize(task.Resolution.x, task.Resolution.y, &sourceRect);
    }

    if (conditionTexture.Width != task.Resolution.x || conditionTexture.Height != task.Resolution.y)
    {
      conditionTexture = conditionTexture.UniformResize(task.Resolution.x, task.Resolution.y, &sourceRect);
    }

    return { move(imageTexture), move(conditionTexture) };
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

    return Tensor::FromTextureData(maskTexture, ColorNormalization::LinearZeroToOne);
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async)
  {
    if (colorImage == _inputImage && _inputLatent) return _inputLatent;

    async.update_state(NAN, "Loading VAE encoder...");
    VaeEncoder vaeEncoder{ *_sessionParameters };

    async.update_state("Encoding color image...");
    _inputLatent = vaeEncoder.EncodeVae(colorImage);
    _inputImage = colorImage;

    return _inputLatent;
  }

  Axodox::MachineLearning::Imaging::StableDiffusion::TextEmbedding StableDiffusionModelExecutor::CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async)
  {
    //Check if the prompt has changed
    if (_textEmbedding && _positivePrompt == task.PositivePrompt && _negativePrompt == task.NegativePrompt && task.SamplingSteps == _stepCount && _isSafeModeEnabled == task.IsSafeModeEnabled) return *_textEmbedding;

    //Load embedder
    async.update_state(NAN, "Loading text embedder...");
    if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_sessionParameters);

    //Parse and schedule prompt
    async.update_state("Creating text embedding...");
    auto encodedNegativePrompt = _textEmbedder->SchedulePrompt((task.IsSafeModeEnabled ? _safetyFilter : "") + task.NegativePrompt, task.SamplingSteps);
    auto encodedPositivePrompt = _textEmbedder->SchedulePrompt(task.PositivePrompt, task.SamplingSteps);

    //Concatenate negative and position prompts
    TextEmbedding textEmbbedding{};
    textEmbbedding.Weights.reserve(encodedNegativePrompt[0].Weights.size() + encodedPositivePrompt[0].Weights.size());
    for (auto weight : encodedNegativePrompt[0].Weights) textEmbbedding.Weights.push_back(-weight);
    for (auto weight : encodedPositivePrompt[0].Weights) textEmbbedding.Weights.push_back(weight);

    ScheduledTensor tensor{ task.SamplingSteps };
    trivial_map<pair<void*, void*>, shared_ptr<EncodedText>> embeddingBuffer;
    for (auto i = 0u; i < task.SamplingSteps; i++)
    {
      auto& concatenatedTensor = embeddingBuffer[{ encodedNegativePrompt[i].Tensor.get(), encodedPositivePrompt[i].Tensor.get() }];
      if (!concatenatedTensor)
      {
        concatenatedTensor = make_shared<EncodedText>(encodedNegativePrompt[i].Tensor->Concat(*encodedPositivePrompt[i].Tensor));
      }

      tensor[i] = concatenatedTensor;
    }

    textEmbbedding.Tensor = move(tensor);
    _textEmbedding = move(textEmbbedding);

    //Update cache key
    _positivePrompt = task.PositivePrompt;
    _negativePrompt = task.NegativePrompt;
    _stepCount = task.SamplingSteps;
    _isSafeModeEnabled = task.IsSafeModeEnabled;

    //Return result
    async.update_state("Text embedding created.");
    return *_textEmbedding;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading denoiser...");
    auto denoiserType = inputs.ConditionImage ? ImageDiffusionInfererKind::ControlNet : ImageDiffusionInfererKind::StableDiffusion;
    if (!_denoiser || _denoiser->Type() != denoiserType || _controlnetId != task.ControlNetMode)
    {
      switch (denoiserType)
      {
      case ImageDiffusionInfererKind::StableDiffusion:
        _denoiser = make_unique<StableDiffusionInferer>(*_sessionParameters);
        break;
      case ImageDiffusionInfererKind::ControlNet:
        _denoiser = make_unique<ControlNetInferer>(_onnxhost->ParametersFromFile(_controlnetRepository->Root() / (task.ControlNetMode + ".onnx")), *_sessionParameters);
        _controlnetId = task.ControlNetMode;
        break;
      }
    }

    StableDiffusionOptions stableDiffusionOptions{
      .StepCount = task.SamplingSteps,
      .BatchSize = task.BatchSize,
      .Width = task.Resolution.x,
      .Height = task.Resolution.y,
      .GuidanceScale = task.GuidanceStrength,
      .Seed = task.RandomSeed,
      .TextEmbeddings = inputs.TextEmbeddings,
      .LatentInput = inputs.InputImage,
      .MaskInput = inputs.InputMask,
      .DenoisingStrength = task.Mode == InferenceMode::Modify ? task.DenoisingStrength : 1.f,
      .Scheduler = task.Scheduler
    };

    async.update_state("Running denoiser...");

    Tensor result;
    if (_denoiser->Type() == ImageDiffusionInfererKind::StableDiffusion)
    {
      result = static_cast<StableDiffusionInferer*>(_denoiser.get())->RunInference(stableDiffusionOptions, &async);
    }
    else
    {
      ControlNetOptions controlNetOptions{ stableDiffusionOptions };
      controlNetOptions.ConditionInput = inputs.ConditionImage;
      controlNetOptions.ConditioningScale = task.ConditioningScale;

      result = static_cast<ControlNetInferer*>(_denoiser.get())->RunInference(controlNetOptions, &async);
    }

    if (!*_unpaintState->IsDenoiserPinned) _denoiser.reset();

    return result;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE decoder...");
    VaeDecoder vaeDecoder{ _sessionParameters->VaeDecoder() };

    async.update_state("Decoding latent image...");
    return vaeDecoder.DecodeVae(latentImage, &async);
  }

  void StableDiffusionModelExecutor::RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& images, Axodox::Threading::async_operation_source& async)
  {
    if (!_sessionParameters->SafetyChecker()) return;

    async.update_state(NAN, "Loading safety checker...");
    SafetyChecker safetyChecker{ *_sessionParameters };

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

  void StableDiffusionModelExecutor::CreateControlNetInpaintCondition(Axodox::MachineLearning::Tensor& condition, const Axodox::Graphics::TextureData& mask, const Axodox::Graphics::Rect& targetRect)
  {
    auto localMask = mask
      .GetTexture(targetRect)
      .UniformResize(uint32_t(condition.Shape[3]), uint32_t(condition.Shape[2]));

    auto pConditionR = condition.AsPointer<float>(0, 0);
    auto pConditionG = condition.AsPointer<float>(0, 1);
    auto pConditionB = condition.AsPointer<float>(0, 2);
    for (auto row = 0u; row < localMask.Height; row++)
    {
      auto pMask = localMask.Row<uint8_t>(row);
      for (auto column = 0u; column < localMask.Width; column++)
      {
        if (*pMask > 127)
        {
          *pConditionR = -1.f;
          *pConditionG = -1.f;
          *pConditionB = -1.f;
        }

        pMask++;
        pConditionR++;
        pConditionG++;
        pConditionB++;
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