#include "pch.h"
#include "TextEmbedder.h"

using namespace std;

namespace Axodox::MachineLearning
{
  const std::set<char> TextEmbedder::_specialChars = { ',', '.', '_', '\'', '"', '-' };

  TextEmbedder::TextEmbedder(OnnxEnvironment& environment, const std::filesystem::path& sourcePath) :
    _textTokenizer(environment, sourcePath),
    _textEncoder(environment)
  { }

  Tensor TextEmbedder::ProcessText(std::string_view text)
  {
    auto chunks = ParseChunks(text.data());

    vector<const char*> texts;
    texts.reserve(chunks.size());
    for (auto& chunk : chunks)
    {
      texts.push_back(chunk.Text.c_str());
    }

    auto tokenizedTexts = _textTokenizer.TokenizeText(texts);
    auto [tokenizedText, attentionMask] = MergeTokenizedChunks(tokenizedTexts, chunks);
    auto encodedText = _textEncoder.EncodeText(tokenizedText);
    ApplyAttention(encodedText, attentionMask);

    return encodedText;
  }

  std::vector<TextChunk> TextEmbedder::ParseChunks(const char* text)
  {
    vector<TextChunk> results;
        
    string textChunk;
    stack<float> attention;
    attention.push(1.f);

    auto addChunk = [&] {
      if (!textChunk.empty())
      {
        results.push_back(TextChunk{
          .Text = move(textChunk),
          .Attention = attention.top()
          });
      }
    };

    while (*text != '\0' && !attention.empty())
    {
      if (isalnum(*text) || isspace(*text) || _specialChars.contains(*text))
      {
        textChunk += *text++;
      }
      else
      {
        switch (*text)
        {
        case '(':
          addChunk();
          text++;

          attention.push(attention.top() * ReadAttention(text));
          break;
        case ')':
          addChunk();
          text++;

          attention.pop();
          break;
        case ':':
          ReadNumber(++text);
          break;
        default:
          text++;
          break;
        }
      }
    }

    addChunk();

    return CleanChunks(results);
  }

  float TextEmbedder::ReadNumber(const char*& text)
  {
    while (isspace(*text)) text++;

    auto start = text;
    while (isdigit(*text) || *text == '.') text++;
    auto end = text;

    while (isspace(*text)) text++;

    float result;
    from_chars(start, end, result);
    return result;
  }

  float TextEmbedder::ReadAttention(const char* text)
  {
    auto level = 0;
    while (*text != '\0')
    {
      switch (*text)
      {
      case '(':
        level++;
        break;
      case ')':
        level--;
        if (level < 0) return 1.f;
        break;
      case ':':
        if (level == 0)
        {
          text++;
          return ReadNumber(text);
        }
      }

      text++;
    }
    return 1.f;
  }

  std::string TextEmbedder::TrimWhitespace(std::string_view text)
  {
    if (text.empty()) return{};

    auto start = text.data();
    auto end = start + text.size();

    while (start != end && isspace(*start)) start++;
    while (start != end && isspace(*(end - 1))) end--;

    return { start, end };
  }

  std::vector<TextChunk> TextEmbedder::CleanChunks(std::vector<TextChunk>& chunks)
  {
    vector<TextChunk> results;
    results.reserve(chunks.size());

    for (auto i = 0; i < chunks.size(); i++)
    {
      auto& chunk = chunks[i];
      chunk.Text = TrimWhitespace(chunk.Text);

      if (!chunk.Text.empty() && chunk.Attention > 0.f)
      {
        if (!results.empty() && results.back().Attention == chunk.Attention)
        {
          results.back().Text += " " + chunk.Text;
        }
        else
        {
          results.push_back(chunk);
        }
      }
    }

    return results;
  }

  std::pair<Tensor, vector<float>> TextEmbedder::MergeTokenizedChunks(const Tensor& tokenizedChunks, std::span<const TextChunk> textChunks)
  {
    Tensor tokenizedTensor{ TensorType::Int32, 1, TextTokenizer::MaxTokenCount };
    auto tokenTarget = tokenizedTensor.AsSpan<int32_t>();
    auto pTokenTarget = tokenTarget.data();

    vector<float> attentionMask;
    attentionMask.resize(TextTokenizer::MaxTokenCount);
    auto pAttention = attentionMask.data();

    auto availableSpace = TextTokenizer::MaxTokenCount;
    for (size_t i = 0; i < tokenizedChunks.Shape[0]; i++)
    {
      auto tokenizedChunk = tokenizedChunks.AsSubSpan<int32_t>(i);
      auto lastToken = tokenizedChunk.end() - 1;
      while (lastToken > tokenizedChunk.begin() && *lastToken == TextTokenizer::BlankToken) lastToken--;

      auto tokensToCopy = size_t(distance(tokenizedChunk.begin(), lastToken)) + 1;
      auto copiableLength = min(tokensToCopy, availableSpace);
      copy(tokenizedChunk.begin(), tokenizedChunk.begin() + copiableLength, pTokenTarget);
      fill(pAttention, pAttention + copiableLength, textChunks[i].Attention);

      pTokenTarget += copiableLength;
      pAttention += copiableLength;
      availableSpace -= copiableLength;
    }

    fill(pTokenTarget, tokenTarget.data() + tokenTarget.size(), TextTokenizer::BlankToken);
    fill(pAttention, attentionMask.data() + attentionMask.size(), 1.f);

    return { tokenizedTensor, attentionMask };
  }

  void TextEmbedder::ApplyAttention(Tensor& encodedText, std::span<const float> attentionMask)
  {
    auto encodedTokens = encodedText.AsSpan<float>();
    auto oldAverage = accumulate(encodedTokens.begin(), encodedTokens.end(), 0.f) / encodedTokens.size();

    for (auto i = 0; i < attentionMask.size(); i++)
    {
      auto encodedToken = encodedText.AsSubSpan<float>(0, i);
      
      auto scale = attentionMask[i];
      for (auto& encodedSubtoken : encodedToken)
      {
        encodedSubtoken *= scale;
      }
    }

    auto newAverage = accumulate(encodedTokens.begin(), encodedTokens.end(), 0.f) / encodedTokens.size();
    auto compensation = oldAverage / newAverage;
    for (auto& encodedSubtoken : encodedTokens)
    {
      encodedSubtoken *= compensation;
    }
  }
}