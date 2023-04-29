#pragma once
#ifdef ONNX
#include "OnnxEnvironment.h"
#include "Tensor.h"

namespace Axodox::MachineLearning
{
  class VaeDecoder
  {
  public:
    VaeDecoder(OnnxEnvironment& environment);

    Tensor DecodeVae(const Tensor& text);

  private:
    OnnxEnvironment& _environment;
    Ort::Session _session;
  };
}
#endif