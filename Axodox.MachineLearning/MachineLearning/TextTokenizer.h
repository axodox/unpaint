#pragma once
#include "OnnxEnvironment.h"
#include "Tensor.h"

namespace Axodox::MachineLearning
{
  class TextTokenizer
  {
  public:
    TextTokenizer(OnnxEnvironment& environment);

    Tensor TokenizeText(const std::string_view text);
    Tensor GetUnconditionalTokens();

  private:
    static const size_t _maxTokenCount;
    static const int32_t _blankToken;

    OnnxEnvironment& _environment;
    Ort::SessionOptions _sessionOptions;
    Ort::Session _session;
  };
}