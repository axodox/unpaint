#pragma once
#include "OnnxEnvironment.h"
#include "Tensor.h"

namespace Axodox::MachineLearning
{
  class VaeEncoder
  {
  public:
    VaeEncoder(OnnxEnvironment& environment);

    Tensor EncodeVae(const Tensor& text);

  private:
    OnnxEnvironment& _environment;
    Ort::Session _session;
  };
}