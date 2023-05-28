#pragma once
#ifdef USE_ONNX
#include "OnnxEnvironment.h"
#include "Tensor.h"
#include "TextTokenizer.h"
#include "TextEncoder.h"
#include "Prompts/PromptAttention.h"

namespace Axodox::MachineLearning
{
  struct TextChunk
  {
    std::string Text;
    float Attention;
  };

  class TextEmbedder
  {
  public:
    TextEmbedder(OnnxEnvironment& environment, const std::filesystem::path& sourcePath = {});

    std::vector<std::shared_ptr<Tensor>> ScheduleText(std::string_view text, uint32_t stepCount);

    Tensor ProcessText(std::string_view text);

  private:
    static const std::set<char> _specialChars;
    TextTokenizer _textTokenizer;
    TextEncoder _textEncoder;

    std::pair<Tensor, std::vector<float>> MergeTokenizedChunks(const Tensor& tokenizedChunks, std::span<const Prompts::PromptAttentionFrame> textChunks);
    void ApplyAttention(Tensor& encodedText, std::span<const float> attentionMask);
  };
}
#endif