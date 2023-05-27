#include "pch.h"
#include "PromptScheduler.h"

using namespace std;

namespace Axodox::MachineLearning
{
  std::vector<std::string> PromptScheduler::SchedulePrompt(std::string_view prompt, uint32_t steps)
  {
    auto textFrames = ParseFrames(prompt);
    auto timeFrames = ConvertTextFramesToTimeFrames(textFrames);

    vector<string> results;
    for (uint32_t i = 0; i < steps; i++)
    {
      auto t = i / float(steps - 1);

      for (auto& frame : timeFrames)
      {
        if (t < frame.Start || t >= frame.End) continue;

        results.push_back(frame.Text);
      }
    }

    return results;
  }

  std::vector<PromptFrame> PromptScheduler::ConvertTextFramesToTimeFrames(std::span<const PromptFrame> frames)
  {
    //Collect key frames
    set<float> keyFrames;
    keyFrames.emplace(0.f);
    keyFrames.emplace(1.f);

    for (auto& frame : frames)
    {
      keyFrames.emplace(frame.Start);
      keyFrames.emplace(frame.End);
    }

    //Build key frames
    vector<PromptFrame> results;
    for (auto it = keyFrames.begin(); it != keyFrames.end(); it++)
    {
      auto keyFrame = *it;
      if (keyFrame == 1.f) break;
      
      PromptFrame timeFrame;

      //Collect prompt
      for (auto& frame : frames)
      {
        if (keyFrame < frame.Start || keyFrame >= frame.End) continue;

        timeFrame.Text += frame.Text;
      }

      //Collect start and end
      timeFrame.Start = keyFrame;
      timeFrame.End = *next(it);

      results.push_back(timeFrame);
    }

    return results;
  }

  std::vector<PromptFrame> PromptScheduler::ParseFrames(std::string_view prompt, float start, float end)
  {
    if (prompt.empty()) return {};

    vector<PromptFrame> results;
    auto stop = prompt.data() + prompt.size();

    const char* segment = nullptr;
    auto text = prompt.data();
    while (text <= stop)
    {
      //Add normal segment
      if (segment && (text == stop || *text == '['))
      {
        results.push_back({ string{ segment, text }, start, end });
        segment = nullptr;
      }

      //Add scheduled segment
      if (*text == '[' && text != stop)
      {
        //Read frame
        float childStart, childEnd;
        string_view childPrompt;
        ReadFrame(text, childStart, childEnd, childPrompt);

        auto childFrames = ParseFrames(childPrompt, childStart, childEnd);
        for (auto& childFrame : childFrames)
        {
          results.push_back({ childFrame.Text, lerp(start, end, childFrame.Start), lerp(start, end, childFrame.End) });
        }
      }
      //Flag unexpected brackets
      else if (*text == ']' && text != stop)
      {
        throw runtime_error("Unexpected closing bracket.");
      }
      //Build normal segments
      else
      {
        if (!segment) segment = text;
        text++;
      }
    }

    return results;
  }

  void PromptScheduler::ReadFrame(const char*& text, float& start, float& end, std::string_view& prompt)
  {
    if (*text != '[') throw runtime_error("Frame must start with bracket.");

    //Split up to segments
    vector<string_view> segments;

    auto depth = 0;
    const char* segment = nullptr;

    do
    {
      switch (*text)
      {
      case '[':
        depth++;
        if (depth == 1)
        {
          segment = text + 1;
        }
        break;
      case '<':
        if (depth == 1)
        {
          segments.push_back({ segment, text });
          segment = text + 1;
        }
        break;
      case ']':
        if (depth == 1)
        {
          segments.push_back({ segment, text });
        }
        depth--;
        break;
      }

      text++;
    } while (*text != '\0' && depth > 0);

    if (depth > 0) throw runtime_error("Unclosed bracket encountered.");

    //Interpret segments
    string_view startSegment, promptSegment, endSegment;
    switch (segments.size())
    {
    case 0: // []
      //Do nothing
      break;
    case 1: // [prompt]
      promptSegment = segments[0];
      break;
    case 2: // [0.2 < prompt] or [ prompt < 0.8]
      if (TryParseNumber(segments[0]))
      {
        startSegment = segments[0];
        promptSegment = segments[1];
      }
      else
      {
        promptSegment = segments[0];
        endSegment = segments[1];
      }
      break;
    case 3: // [0.2 < prompt < 0.8]
      startSegment = segments[0];
      promptSegment = segments[1];
      endSegment = segments[2];
      break;
    default:
      throw runtime_error("Too many segments in a prompt frame.");
    }

    //Parse segments
    auto startValue = TryParseNumber(startSegment);
    if (!startSegment.empty() && !startValue) throw runtime_error("Could not parse frame start.");
    start = startValue ? *startValue : 0.f;

    auto endValue = TryParseNumber(endSegment);
    if (!endSegment.empty() && !endValue) throw runtime_error("Could not parse frame end.");
    end = endValue ? *endValue : 1.f;

    prompt = promptSegment;
  }

  std::optional<float> PromptScheduler::TryParseNumber(std::string_view text)
  {
    if (text.empty()) return nullopt;

    //Skip whitespace
    auto start = text.data();
    auto end = text.data() + text.size();

    while (isspace(*start) && start < end) start++;
    while (isspace(*(end - 1)) && start < end) end--;

    //Parse body
    float result;
    auto parseResult = from_chars(start, end, result);
    if (parseResult.ec == errc() && parseResult.ptr == end)
    {
      return result;
    }
    else
    {
      return nullopt;
    }
  }
}