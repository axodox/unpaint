#pragma once
#include "pch.h"
#include "Graphics/Textures/TextureData.h"
#include "Threading/AsyncOperation.h"
#include "MachineLearning/Tensor.h"
#include "MachineLearning/OnnxEnvironment.h"
#include "MachineLearning/StableDiffustionInferer.h"
#include "MachineLearning/TextEmbedder.h"
#include "ModelRepository.h"
#include "ControlNetRepository.h"
#include "UnpaintState.h"
#include "ImageMetadata.h"

namespace winrt::Unpaint
{
  struct StableDiffusionInferenceTask
  {
    static const char* PositivePromptPlaceholder;
    static const char* NegativePromptPlaceholder;

    InferenceMode Mode;

    std::string PositivePrompt, NegativePrompt;
    Axodox::MachineLearning::StableDiffusionSchedulerKind Scheduler;
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
      Axodox::MachineLearning::TextEmbedding TextEmbeddings;
    };

    struct ModelFile
    {
      std::filesystem::path ModelPath;
      std::vector<uint8_t> ModelData;

      operator Axodox::MachineLearning::ModelSource() const;
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

    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> _onnxEnvironment;
    std::unique_ptr<Axodox::MachineLearning::TextEmbedder> _textEmbedder;
    std::unique_ptr<Axodox::MachineLearning::ImageDiffusionInferer> _denoiser;
    std::mutex _mutex;

    std::string _modelId;
    std::unordered_map<std::string, winrt::Windows::Storage::StorageFile> _modelFiles;

    uint32_t _stepCount;
    bool _isSafeModeEnabled;
    std::string _positivePrompt, _negativePrompt;
    std::optional<Axodox::MachineLearning::TextEmbedding> _textEmbedding;

    Axodox::MachineLearning::Tensor _inputImage, _inputLatent;

    void EnsureEnvironment(std::string_view modelId);
    ModelFile GetModelFile(const std::string& fileId) const;

    std::pair<Axodox::Graphics::TextureData, Axodox::Graphics::TextureData> LoadImage(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor LoadMask(const StableDiffusionInferenceTask& task, Axodox::Graphics::Rect& sourceRect, Axodox::Graphics::Rect& targetRect, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor EncodeVAE(const Axodox::MachineLearning::Tensor& colorImage, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::TextEmbedding CreateTextEmbeddings(const StableDiffusionInferenceTask& task, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor RunStableDiffusion(const StableDiffusionInferenceTask& task, const StableDiffusionInputs& inputs, Axodox::Threading::async_operation_source& async);
    Axodox::MachineLearning::Tensor DecodeVAE(const Axodox::MachineLearning::Tensor& latentImage, Axodox::Threading::async_operation_source& async);
    void RunSafetyCheck(std::vector<Axodox::Graphics::TextureData>& results, Axodox::Threading::async_operation_source& async);

    void CreateControlNetInpaintCondition(Axodox::MachineLearning::Tensor& condition, const Axodox::Graphics::TextureData& mask, const Axodox::Graphics::Rect& targetRect);
  };
}