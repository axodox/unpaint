#pragma once

namespace Axodox::MachineLearning::Prompts
{
  struct PromptAttentionFrame
  {
    std::string Text;
    float Attention;

    bool operator==(const PromptAttentionFrame&) const = default;
  };

  std::vector<PromptAttentionFrame> ParseAttentionFrames(std::string_view prompt, float attention = 1.f);
}
