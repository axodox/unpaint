#include "pch.h"
#include "LMSDiscreteScheduler.h"

using namespace std;

namespace Axodox::MachineLearning
{
  const int LmsDiscreteSchedulerSteps::DerivativeOrder = 4;

  LmsDiscreteScheduler::LmsDiscreteScheduler(const LmsDiscreteSchedulerOptions& options) :
    _options(options)
  {
    auto betas = _options.BetasTrained;

    if (betas.empty())
    {
      switch (_options.BetaSchedulerType)
      {
      case LmsBetaSchedulerKind::Linear:
        betas = GetLinearBetas();
        break;
      case LmsBetaSchedulerKind::ScaledLinear:
        betas = GetScaledLinearBetas();
        break;
      default:
        throw logic_error("LmsBetaSchedulerKind not implemented.");
      }
    }
    else
    {
      if (betas.size() != _options.TrainStepCount) throw invalid_argument("options.BetasTrained.Size() != options.TrainStepCount");
    }

    _cumulativeAlphas = CalculateCumulativeAlphas(betas);
    _initialNoiseSigma = CalculateInitialNoiseSigma(_cumulativeAlphas);
  }

  float IntegrateOverInterval(const std::function<float(float)>& f, float intervalStart, float intervalEnd)
  {
    auto stepCount = 100;
    auto stepSize = (intervalEnd - intervalStart) / stepCount;

    auto result = 0.f;
    for (auto value = intervalStart; value < intervalEnd; value += stepSize)
    {
      result += f(value) * stepSize;
    }
    return result;
  }

  LmsDiscreteSchedulerSteps LmsDiscreteScheduler::GetSteps(size_t count) const
  {
    //Calculate timesteps
    vector<float> timesteps;
    timesteps.resize(count);

    auto step = (_options.TrainStepCount - 1) / float(count - 1);
    for (auto value = 0.f; auto & timestep : timesteps)
    {
      timestep = value;
      value += step;
    }

    //Calculate sigmas
    vector<float> sigmas{ _cumulativeAlphas.rbegin(), _cumulativeAlphas.rend() };
    for (auto& sigma : sigmas)
    {
      sigma = sqrt((1.f - sigma) / sigma);
    }

    vector<float> interpolatedSigmas;
    interpolatedSigmas.reserve(count);
    interpolatedSigmas.resize(count);
    for (size_t i = 0; auto & interpolatedSigma : interpolatedSigmas)
    {
      auto trainStep = timesteps[i++];
      auto previousIndex = max(size_t(floor(trainStep)), size_t(0));
      auto nextIndex = min(size_t(ceil(trainStep)), sigmas.size() - 1);
      interpolatedSigma = lerp(sigmas[previousIndex], sigmas[nextIndex], trainStep - floor(trainStep));
    }
    interpolatedSigmas.push_back(0.f);
    ranges::reverse(timesteps);

    //Calculate LMS coefficients
    vector<vector<float>> lmsCoefficients;
    for (auto i = 0; i < count; i++)
    {
      auto order = min(i + 1, LmsDiscreteSchedulerSteps::DerivativeOrder);

      vector<float> coefficients;
      for (auto j = 0; j < order; j++)
      {
        auto lmsDerivative = [&](float tau) {
          float product = 1.f;
          for (auto k = 0; k < order; k++)
          {
            if (j == k)
            {
              continue;
            }
            product *= (tau - interpolatedSigmas[i - k]) / (interpolatedSigmas[i - j] - interpolatedSigmas[i - k]);
          }
          return product;
        };

        coefficients.push_back(-IntegrateOverInterval(lmsDerivative, interpolatedSigmas[i + 1], interpolatedSigmas[i]));
      }

      lmsCoefficients.push_back(move(coefficients));
    }

    //Return result
    LmsDiscreteSchedulerSteps result;
    result.Timesteps = move(timesteps);
    result.Sigmas = move(interpolatedSigmas);
    result.LmsCoefficients = move(lmsCoefficients);
    return result;
  }

  float LmsDiscreteScheduler::InitialNoiseSigma() const
  {
    return _initialNoiseSigma;
  }

  std::vector<float> LmsDiscreteScheduler::GetLinearBetas() const
  {
    vector<float> results;
    results.resize(_options.TrainStepCount);

    auto value = _options.BetaAtStart;
    auto step = (_options.BetaAtEnd - _options.BetaAtStart) / (_options.TrainStepCount - 1.f);
    for (auto& beta : results)
    {
      beta = value;
      value += step;
    }

    return results;
  }

  std::vector<float> LmsDiscreteScheduler::GetScaledLinearBetas() const
  {
    vector<float> results;
    results.resize(_options.TrainStepCount);

    auto value = sqrt(_options.BetaAtStart);
    auto step = (sqrt(_options.BetaAtEnd) - value) / (_options.TrainStepCount - 1.f);
    for (auto& beta : results)
    {
      beta = value * value;
      value += step;
    }

    return results;
  }
  
  std::vector<float> LmsDiscreteScheduler::CalculateCumulativeAlphas(std::span<const float> betas)
  {
    vector<float> results{ betas.begin(), betas.end() };

    float value = 1.f;
    for (auto& result : results)
    {
      value *= 1.f - result;
      result = value;
    }

    return results;
  }
  
  float LmsDiscreteScheduler::CalculateInitialNoiseSigma(std::span<const float> cumulativeAlphas)
  {
    float result = 0;

    for (auto cumulativeAlpha : cumulativeAlphas)
    {
      auto sigma = sqrt((1.f - cumulativeAlpha) / cumulativeAlpha);
      if (sigma > result) result = sigma;
    }

    return result;
  }
  
  Tensor LmsDiscreteSchedulerSteps::ApplyStep(const Tensor& latents, const Tensor& noise, list<Tensor>& derivatives, size_t step)
  {
    auto sigma = Sigmas[step];

    //Compute predicted original sample (x_0) from sigma-scaled predicted noise
    auto predictedOriginalSample = latents.BinaryOperation<float>(noise, [sigma](float a, float b) { return a - sigma * b; });

    //Convert to an ODE derivative
    auto currentDerivative = latents.BinaryOperation<float>(predictedOriginalSample, [sigma](float a, float b) { return (a - b) / sigma; });

    derivatives.push_back(currentDerivative);
    if (derivatives.size() > DerivativeOrder) derivatives.pop_front();

    //Compute linear multistep coefficients
    auto& lmsCoefficients = LmsCoefficients[step];

    //Compute previous sample based on the derivative path
    vector<Tensor> lmsDerivativeProduct;
    lmsDerivativeProduct.reserve(derivatives.size());
    for (auto i = 0; auto& derivative : ranges::reverse_view(derivatives))
    {
      //Multiply to coeff by each derivatives to create the new tensors
      lmsDerivativeProduct.push_back(derivative * lmsCoefficients[i++]);
    }

    //Sum the tensors
    Tensor lmsDerivativeSum{ TensorType::Single, currentDerivative.Shape };
    for (auto& tensor : lmsDerivativeProduct)
    {
      lmsDerivativeSum.UnaryOperation<float>(tensor, [](float a, float b) { return a + b; });
    }

    //Add the sumed tensor to the sample
    return latents.BinaryOperation<float>(lmsDerivativeSum, [](float a, float b) { return a + b; });
  }
}