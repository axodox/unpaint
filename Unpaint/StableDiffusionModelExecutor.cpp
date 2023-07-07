#include "pch.h"
#include "StableDiffusionModelExecutor.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"
#include "Storage/FileIO.h"
#include "Storage/UwpStorage.h"
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
  const char* StableDiffusionInferenceTask::PositivePromptPlaceholder = "an empty canvas standing in a painter's workshop";
  const char* StableDiffusionInferenceTask::NegativePromptPlaceholder = "blurry, render";

  const char* const StableDiffusionModelExecutor::_safetyFilter = "nsfw, nudity, porn, sex, child, girl, boy, minor, teen, ";

  StableDiffusionModelExecutor::StableDiffusionModelExecutor() :
    _unpaintState(dependencies.resolve<UnpaintState>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _controlnetRepository(dependencies.resolve<ControlNetRepository>()),
    _stepCount(0),
    _isSafeModeEnabled(true)
  { }

  int32_t StableDiffusionModelExecutor::ValidatePrompt(std::string_view modelId, std::string prompt, bool isSafeModeEnabled)
  {
    try
    {
      lock_guard lock(_mutex);
      EnsureEnvironment(modelId);
      if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_onnxEnvironment, app_folder(), GetModelFile("text_encoder\\model.onnx"));

      if (isSafeModeEnabled) prompt = _safetyFilter + prompt;

      return _textEmbedder->ValidatePrompt(prompt);
    }
    catch (...)
    {
      return -1;
    }
  }

  std::vector<Axodox::Graphics::TextureData> StableDiffusionModelExecutor::TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation)
  {
    //Set up async source
    lock_guard lock(_mutex);
    thread_name_context threadName{ "* inference" };

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
      if (task.InputImage || task.InputCondition)
      {
        inputs.InputMask = LoadMask(task, sourceRect, targetRect, async);

        auto [imageTexture, conditionTexture] = LoadImage(task, sourceRect, targetRect, async);
        if (task.InputImage && task.DenoisingStrength < 1.f)
        {
          targetTexture = TextureData{ task.InputImage };
          inputs.InputImage = EncodeVAE(Tensor::FromTextureData(imageTexture, ColorNormalization::LinearPlusMinusOne), async);
        }

        if (task.InputCondition)
        {
          inputs.ConditionImage = Tensor::FromTextureData(conditionTexture, ColorNormalization::LinearZeroToOne);
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
    if (!_onnxEnvironment || (*_onnxEnvironment)->DeviceId != int32_t(*_unpaintState->AdapterIndex) || _modelId != modelId)
    {
      _textEmbedder.reset();
      _denoiser.reset();

      auto host = dependencies.resolve<OnnxHost>();
      _onnxEnvironment = make_unique<OnnxEnvironment>(host, _modelRepository->Root() / modelId);
      (*_onnxEnvironment)->DeviceId = *_unpaintState->AdapterIndex;
      _modelId = modelId;
      _modelFiles = _modelRepository->GetModelFiles(modelId);

      _stepCount = 0;
      _positivePrompt.clear();
      _negativePrompt.clear();
      _textEmbedding.clear();

      _inputImage = {};
      _inputLatent = {};
    }
  }

  StableDiffusionModelExecutor::ModelFile StableDiffusionModelExecutor::GetModelFile(const std::string& fileId) const
  {
    auto file = _modelFiles.at(fileId);

    ModelFile result;
    result.ModelPath = filesystem::path(file.Path().c_str());
    
    if (!wstring(file.Path().c_str()).starts_with(_modelRepository->Root().c_str()))
    {
      result.ModelData = read_file(file);
    }

    return result;
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
    VaeEncoder vaeEncoder{ *_onnxEnvironment, GetModelFile("vae_encoder\\model.onnx") };

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
    if (!_textEmbedder) _textEmbedder = make_unique<TextEmbedder>(*_onnxEnvironment, app_folder(), GetModelFile("text_encoder\\model.onnx"));

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
    auto denoiserType = inputs.ConditionImage ? ImageDiffusionInfererKind::ControlNet : ImageDiffusionInfererKind::StableDiffusion;
    if (!_denoiser || _denoiser->Type() != denoiserType)
    {
      switch (denoiserType)
      {
      case ImageDiffusionInfererKind::StableDiffusion:
        _denoiser = make_unique<StableDiffusionInferer>(*_onnxEnvironment, GetModelFile("unet\\model.onnx"));
        break;
      case ImageDiffusionInfererKind::ControlNet:
        _denoiser = make_unique<ControlNetInferer>(*_onnxEnvironment, _controlnetRepository->Root(), GetModelFile("controlnet\\model.onnx"));
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
      .DenoisingStrength = task.Mode == InferenceMode::Modify ? task.DenoisingStrength : 1.f
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
      controlNetOptions.ConditionType = task.ControlNetMode;
      controlNetOptions.ConditioningScale = task.ConditioningScale;

      result = static_cast<ControlNetInferer*>(_denoiser.get())->RunInference(controlNetOptions, &async);
    }

    if (!*_unpaintState->IsDenoiserPinned) _denoiser.reset();

    return result;
  }

  Axodox::MachineLearning::Tensor StableDiffusionModelExecutor::DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading VAE decoder...");
    VaeDecoder vaeDecoder{ *_onnxEnvironment, GetModelFile("vae_decoder\\model.onnx") };

    async.update_state("Decoding latent image...");
    return vaeDecoder.DecodeVae(latentImage);
  }

  void StableDiffusionModelExecutor::RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& images, Axodox::Threading::async_operation_source& async)
  {
    async.update_state(NAN, "Loading safety checker...");
    SafetyChecker safetyChecker{ *_onnxEnvironment, GetModelFile("safety_checker\\model.onnx") };

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

  StableDiffusionModelExecutor::ModelFile::operator Axodox::MachineLearning::ModelSource() const
  {
    if (ModelData.empty())
    {
      return ModelPath;
    }
    else
    {
      return ModelData;
    }
  }
}