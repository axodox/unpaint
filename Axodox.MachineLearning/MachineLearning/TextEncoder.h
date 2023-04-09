#pragma once
#include "OnnxEnvironment.h"
#include "Tensor.h"

namespace Axodox::MachineLearning
{
  class TextEncoder
  {
  public:
    TextEncoder(OnnxEnvironment& environment);

    Tensor EncodeText(Tensor text);

  private:
    static const size_t _maxTokenCount;

    OnnxEnvironment& _environment;
    Ort::Session _session;
  };
}