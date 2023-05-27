#pragma once
#include "pch.h"

namespace Axodox::MachineLearning
{
  struct PromptFrame
  {
    std::string Text;
    float Start;
    float End;

    bool operator==(const PromptFrame&) const = default;
  };

  class PromptScheduler
  {
  public:
    static std::vector<std::string> SchedulePrompt(std::string_view prompt, uint32_t steps);

    static std::vector<PromptFrame> ConvertTextFramesToTimeFrames(std::span<const PromptFrame> frames);

    static std::vector<PromptFrame> ParseFrames(std::string_view prompt, float start = 0.f, float end = 1.f);

  private:
    static void ReadFrame(const char*& text, float& start, float& end, std::string_view& prompt);

    static std::optional<float> TryParseNumber(std::string_view text);
  };
}