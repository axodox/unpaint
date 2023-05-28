#pragma once
#include "pch.h"

namespace Axodox::MachineLearning::Prompts
{
  struct PromptTimeFrame
  {
    std::string Text;
    float Start;
    float End;

    bool operator==(const PromptTimeFrame&) const = default;
  };

  std::vector<PromptTimeFrame> ParseTimeFrames(std::string_view prompt, float start = 0.f, float end = 1.f);

  std::vector<PromptTimeFrame> SchedulePrompt(std::string_view prompt);
  std::vector<std::string> SchedulePrompt(std::string_view prompt, uint32_t steps);
}