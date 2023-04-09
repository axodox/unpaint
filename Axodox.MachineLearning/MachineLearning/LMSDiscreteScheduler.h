#pragma once
#include "Tensor.h"

namespace Axodox::MachineLearning
{
  enum class LmsBetaSchedulerKind
  {
    ScaledLinear,
    Linear
  };

  enum class LmsPredictorKind
  {
    Epsilon,
    //VPrediction
  };

  struct LmsDiscreteSchedulerOptions
  {
    size_t TrainStepCount = 1000;
    float BetaAtStart = 0.00085f;
    float BetaAtEnd = 0.012f;
    LmsBetaSchedulerKind BetaSchedulerType = LmsBetaSchedulerKind::ScaledLinear;
    LmsPredictorKind PredictorType = LmsPredictorKind::Epsilon;
    std::vector<float> BetasTrained;
  };

  struct LmsDiscreteSchedulerSteps
  {
    static const int DerivativeOrder;

    std::vector<float> Timesteps;
    std::vector<float> Sigmas;
    std::vector<std::vector<float>> LmsCoefficients;

    Tensor ApplyStep(const Tensor& latents, const Tensor& noise, std::list<Tensor>& derivatives, size_t step);
  };

  class LmsDiscreteScheduler
  {
  public:
    LmsDiscreteScheduler(const LmsDiscreteSchedulerOptions& options = {});

    LmsDiscreteSchedulerSteps GetSteps(size_t count) const;

    float InitialNoiseSigma() const;

  private:
    LmsDiscreteSchedulerOptions _options;
    std::vector<float> _cumulativeAlphas;
    float _initialNoiseSigma;

    std::vector<float> GetLinearBetas() const;
    std::vector<float> GetScaledLinearBetas() const;
    static std::vector<float> CalculateCumulativeAlphas(std::span<const float> betas);
    static float CalculateInitialNoiseSigma(std::span<const float> cumulativeAlphas);
  };
}