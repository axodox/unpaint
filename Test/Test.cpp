// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "UI/Window.h"
#include "Infrastructure/Events.h"
#include "Graphics/Swap Chains/HwndSwapChain.h"
#include "Storage/FileStream.h"
#include "Storage/FileIO.h"

#include "MachineLearning/SafetyChecker.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::UI;
using namespace Axodox::Storage;
using namespace Axodox::MachineLearning;
using namespace std;
using namespace winrt;


int main()
{
  winrt::init_apartment();

  OnnxEnvironment onnxEnvironment{ L"D:/dev/realistic_vision_onnx" };

  SafetyChecker safetyChecker{ onnxEnvironment };

  filesystem::path sourceDirectory{ "F:/Images/unpaint" };
  filesystem::path safeDirectory{ "F:/Images/unpaint/safe" };
  filesystem::path unsafeDirectory{ "F:/Images/unpaint/unsafe" };

  filesystem::create_directories(safeDirectory);
  filesystem::create_directories(unsafeDirectory);

  for (auto item : filesystem::directory_iterator{ sourceDirectory })
  {
    if (item.is_directory() || item.path().extension() != ".png") continue;

    printf("Checking %s...", to_string(item.path().filename().wstring()).c_str());

    auto imageBuffer = try_read_file(item.path());
    auto imageData = TextureData::FromBuffer(imageBuffer);
    if (safetyChecker.IsSafe(imageData))
    {
      printf(" Safe.\n");
      filesystem::copy_file(item.path(), safeDirectory / item.path().filename());
    }
    else
    {
      printf(" Not safe.\n");
      filesystem::copy_file(item.path(), unsafeDirectory / item.path().filename());
    }
  }

  printf("Done.\n");
  return 0;
}