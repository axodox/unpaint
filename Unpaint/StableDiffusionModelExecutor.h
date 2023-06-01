#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "Threading/AsyncOperation.h"
#include "MachineLearning/Tensor.h"
#include "MachineLearning/OnnxEnvironment.h"
#include "MachineLearning/StableDiffustionInferer.h"
#include "MachineLearning/TextEmbedder.h"
#include "ModelRepository.h"
#include "UnpaintOptions.h"
#include "ImageMetadata.h"

namespace winrt::Unpaint
{
  enum class InferenceMode
  {
    Create,
    Modify
  };

  struct StableDiffusionInferenceTask
  {
    InferenceMode Mode;

    std::string PositivePrompt, NegativePrompt;
    DirectX::XMUINT2 Resolution;

    float GuidanceStrength;
    float DenoisingStrength;
    uint32_t SamplingSteps;
    uint32_t RandomSeed;
    uint32_t BatchSize;
    bool SafeMode;
    bool IsSafetyCheckerEnabled;

    std::string ModelId;
    std::filesystem::path InputImage;
    Axodox::Graphics::TextureData InputMask;

    ImageMetadata ToMetadata() const;
  };

  class StableDiffusionModelExecutor
  {
    struct StableDiffusionInputs
    {
      Axodox::MachineLearning::Tensor InputImage;
      Axodox::MachineLearning::Tensor InputMask;
      Axodox::MachineLearning::ScheduledTensor TextEmbeddings;
    };

  public:
    StableDiffusionModelExecutor();

    int32_t ValidatePrompt(std::string_view modelId, std::string_view prompt);

    std::vector<Axodox::Graphics::TextureData> TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation);

  private:
    static const char* const _safetyFilter;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;

    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _onnxEnvironment;
    std::unique_ptr<Axodox::MachineLearning::TextEmbedder> _textEmbedder;
    std::unique_ptr<Axodox::MachineLearning::StableDiffusionInferer> _denoiser;
    std::mutex _mutex;

    std::string _modelId;

    uint32_t _stepCount;
    std::string _positivePrompt, _negativePrompt;
    Axodox::MachineLearning::ScheduledTensor _textEmbedding;

    Axodox::MachineLearning::Tensor _inputImage, _inputLatent;

    void EnsureEnvironment(std::string_view modelId);

    Axodox::MachineLearning::Tensor LoadImage(const StableDiffusionInferenceTask& task, Axodox::Graphics::TextureData& sourceTexture, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor LoadMask(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::ScheduledTensor CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async);
    void RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& results, Axodox::Threading::async_operation_source& async);
  };
}