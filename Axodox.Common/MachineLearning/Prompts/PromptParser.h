#pragma once
#include "pch.h"

namespace Axodox::MachineLearning::Prompts
{
  std::pair<const char*, const char*> ToRange(std::string_view text);

  std::string_view TrimWhitespace(std::string_view text);

  std::optional<float> TryParseNumber(std::string_view text);

  std::vector<std::string_view> SplitToSegments(const char*& text, char opener, char delimiter, char closer);

  void CheckPromptCharacters(std::string_view text);
}