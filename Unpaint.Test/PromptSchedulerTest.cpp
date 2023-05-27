#include "pch.h"
#include "CppUnitTest.h"
#include "MachineLearning/PromptScheduler.h"

using namespace Axodox::MachineLearning;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnpaintTest
{
  TEST_CLASS(PromptSchedulerTest)
  {
  public:
    TEST_METHOD(TestEmptyPrompt)
    {
      auto result = PromptScheduler::ParseFrames("");
      Assert::IsTrue(result == vector<PromptFrame>{});
    }

    TEST_METHOD(TestBasicPrompt)
    {
      auto result = PromptScheduler::ParseFrames("test");
      Assert::IsTrue(result == vector<PromptFrame>{
        {"test", 0.f, 1.f}
      });
    }

    TEST_METHOD(TestEmptyBrackets)
    {
      auto result = PromptScheduler::ParseFrames("test[]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test", 0.f, 1.f }
      });
    }

    TEST_METHOD(TestFromPrompt)
    {
      auto result = PromptScheduler::ParseFrames("test[0.2<this]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test", 0.f, 1.f },
        { "this", 0.2f, 1.f }
      });
    }

    TEST_METHOD(TestToPrompt)
    {
      auto result = PromptScheduler::ParseFrames("test [this < 0.8 ]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test ", 0.f, 1.f },
        { "this ", 0.f, 0.8f }
      });
    }

    TEST_METHOD(TestFromToPrompt)
    {
      auto result = PromptScheduler::ParseFrames("test [0.2<this < 0.8 ]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test ", 0.f, 1.f },
        { "this ", 0.2f, 0.8f }
      });
    }

    TEST_METHOD(TestTwoPrompts)
    {
      auto result = PromptScheduler::ParseFrames("test [0.2<this < 0.8 ] at [0.2<once]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test ", 0.f, 1.f },
        { "this ", 0.2f, 0.8f },
        { " at ", 0.f, 1.f },
        { "once", 0.2f, 1.f }
      });
    }

    TEST_METHOD(TestTightPrompts)
    {
      auto result = PromptScheduler::ParseFrames("test [0.2<this at< 0.8 ][0.2<once]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test ", 0.f, 1.f },
        { "this at", 0.2f, 0.8f },
        { "once", 0.2f, 1.f }
      });
    }

    TEST_METHOD(TestNestedPrompts)
    {
      auto result = PromptScheduler::ParseFrames("test [0.2<this [0.5<at]< 0.8 ][0.2<once]");
      Assert::IsTrue(result == vector<PromptFrame>{
        { "test ", 0.f, 1.f },
        { "this ", 0.2f, 0.8f },
        { "at", 0.5f, 0.8f },
        { "once", 0.2f, 1.f }
      });
    }

    TEST_METHOD(TestUnclosedBracket)
    {
      try
      {
        PromptScheduler::ParseFrames("test [ asd");
        Assert::Fail();
      }
      catch(...)
      { }
    }

    TEST_METHOD(TestUnexpectedClosedBracket)
    {
      try
      {
        PromptScheduler::ParseFrames("test [ asd]]");
        Assert::Fail();
      }
      catch (...)
      { }
    }

    TEST_METHOD(TestBadSegments)
    {
      try
      {
        PromptScheduler::ParseFrames("test [ asd<asd]");
        Assert::Fail();
      }
      catch (...)
      { }
    }

    TEST_METHOD(TestTooManySegments)
    {
      try
      {
        PromptScheduler::ParseFrames("test [ 0.1<0.2<0.3<0.4]");
        Assert::Fail();
      }
      catch (...)
      { }
    }
  };
}
