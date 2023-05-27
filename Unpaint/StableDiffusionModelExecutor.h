#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "Threading/AsyncOperation.h"
#include "MachineLearning/Tensor.h"
#include "MachineLearning/OnnxEnvironment.h"
#include "MachineLearning/StableDiffustionInferer.h"
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

    std::string ModelId;
    std::filesystem::path InputImage;

    ImageMetadata ToMetadata() const;
  };

  class StableDiffusionModelExecutor
  {
    struct StableDiffusionInputs
    {
      Axodox::MachineLearning::Tensor InputImage;
      Axodox::MachineLearning::Tensor InputMask;
      Axodox::MachineLearning::Tensor TextEmbeddings;
    };

  public:
    StableDiffusionModelExecutor();

    std::vector<Axodox::Graphics::TextureData> TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation);

  private:
    static const char* const _safetyFilter;
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;

    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _onnxEnvironment;
    std::unique_ptr<Axodox::MachineLearning::StableDiffusionInferer> _denoiser;
    std::mutex _mutex;

    std::string _modelId;

    std::string _positivePrompt, _negativePrompt;
    Axodox::MachineLearning::Tensor _textEmbedding;

    Axodox::MachineLearning::Tensor LoadImage(const StableDiffusionInferenceTask& task, std::optional<DirectX::XMUINT2>& resolutionOverride, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async);
  };
}