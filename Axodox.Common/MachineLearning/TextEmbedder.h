#pragma once
#ifdef USE_ONNX
#include "OnnxEnvironment.h"
#include "Tensor.h"
#include "TextTokenizer.h"
#include "TextEncoder.h"

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

    Tensor ProcessText(std::string_view text);


  private:
    static const std::set<char> _specialChars;
    TextTokenizer _textTokenizer;
    TextEncoder _textEncoder;

    std::vector<TextChunk> ParseChunks(const char* text);
    float ReadNumber(const char*& text);
    float ReadAttention(const char* text);

    std::string TrimWhitespace(std::string_view text);
    std::vector<TextChunk> CleanChunks(std::vector<TextChunk>& chunks);

    std::pair<Tensor, std::vector<float>> MergeTokenizedChunks(const Tensor& tokenizedChunks, std::span<const TextChunk> textChunks);
    void ApplyAttention(Tensor& encodedText, std::span<const float> attentionMask);
  };
}
#endif