#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "ModelRepository.h"
#include "ControlNetRepository.h"
#include "UnpaintState.h"
#include "ImageMetadata.h"
#include "OnnxHost.h"

namespace winrt::Unpaint
{
  struct StableDiffusionInferenceTask
  {
    static const char* PositivePromptPlaceholder;
    static const char* NegativePromptPlaceholder;

    InferenceMode Mode;

    std::string PositivePrompt, NegativePrompt;
    Axodox::MachineLearning::Imaging::StableDiffusion::Schedulers::StableDiffusionSchedulerKind Scheduler;
    DirectX::XMUINT2 Resolution;

    float GuidanceStrength;
    float DenoisingStrength;
    uint32_t SamplingSteps;
    uint32_t RandomSeed;
    uint32_t BatchSize;
    bool IsSafeModeEnabled;
    bool IsSafetyCheckerEnabled;

    std::string ModelId;
    Axodox::Graphics::TextureData InputImage;
    Axodox::Graphics::TextureData InputMask;
    Axodox::Graphics::TextureData InputCondition;

    std::string ControlNetMode;
    float ConditioningScale;

    ImageMetadata ToMetadata() const;
  };

  class StableDiffusionModelExecutor
  {
    struct StableDiffusionInputs
    {
      Axodox::MachineLearning::Tensor InputImage;
      Axodox::MachineLearning::Tensor ConditionImage;
      Axodox::MachineLearning::Tensor InputMask;
      Axodox::MachineLearning::Imaging::StableDiffusion::TextEmbedding TextEmbeddings;
    };

  public:
    StableDiffusionModelExecutor();

    int32_t ValidatePrompt(std::string_view modelId, std::string prompt, bool isSafeModeEnabled);

    std::vector<Axodox::Graphics::TextureData> TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation);

  private:
    static const char* const _safetyFilter;
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<ModelRepository> _modelRepository;
    std::shared_ptr<ControlNetRepository> _controlnetRepository;
    std::shared_ptr<OnnxHost> _onnxhost;

    std::unique_ptr<Axodox::MachineLearning::Imaging::StableDiffusion::TextEmbedder> _textEmbedder;
    std::unique_ptr<Axodox::MachineLearning::Imaging::StableDiffusion::ImageDiffusionInferer> _denoiser;
    std::mutex _mutex;

    std::string _modelId, _controlnetId;
    std::unique_ptr<StableDiffusionStorageFileMapSessionParameters> _sessionParameters;

    uint32_t _stepCount;
    bool _isSafeModeEnabled;
    std::string _positivePrompt, _negativePrompt;
    std::optional<Axodox::MachineLearning::Imaging::StableDiffusion::TextEmbedding> _textEmbedding;

    Axodox::MachineLearning::Tensor _inputImage, _inputLatent;

    void EnsureEnvironment(std::string_view modelId);

    std::pair<Axodox::Graphics::TextureData, Axodox::Graphics::TextureData> LoadImage(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor LoadMask(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Imaging::StableDiffusion::TextEmbedding CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async);
    void RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& results, Axodox::Threading::async_operation_source& async);

    void CreateControlNetInpaintCondition(Axodox::MachineLearning::Tensor& condition, const Axodox::Graphics::TextureData& mask, const Axodox::Graphics::Rect& targetRect);
  };
}