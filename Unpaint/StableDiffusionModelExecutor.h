#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "Threading/AsyncOperation.h"
#include "MachineLearning/Tensor.h"
#include "MachineLearning/OnnxEnvironment.h"
#include "MachineLearning/StableDiffustionInferer.h"
#include "ModelRepository.h"
#include "UnpaintOptions.h"

namespace winrt::Unpaint
{
  struct StableDiffusionInferenceTask
  {
    std::string PositivePrompt, NegativePrompt;
    DirectX::XMUINT2 Resolution;

    float GuidanceStrength;
    uint32_t SamplingSteps;
    uint32_t RandomSeed;

    std::string ModelId;
  };

  class StableDiffusionModelExecutor
  {
  public:
    StableDiffusionModelExecutor();

    Axodox::MachineLearning::Tensor TryRunInference(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation& operation);

  private:
    std::shared_ptr<UnpaintOptions> _unpaintOptions;
    std::shared_ptr<ModelRepository> _modelRepository;

    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _onnxEnvironment;
    std::unique_ptr<Axodox::MachineLearning::StableDiffusionInferer> _denoiser;
    std::mutex _mutex;

    std::string _modelId;

    std::string _positivePrompt, _negativePrompt;
    Axodox::MachineLearning::Tensor _textEmbedding;

    Axodox::MachineLearning::Tensor CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor RunStableDiffusion(const StableDiffusionInferenceTask& task, const Axodox::MachineLearning::Tensor& textEmbeddings, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async);
  };
}