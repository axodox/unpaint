// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "UI/Window.h"
#include "Infrastructure/Events.h"
#include "Graphics/Swap Chains/HwndSwapChain.h"
#include "Storage/FileStream.h"
#include "Storage/FileIO.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::UI;
using namespace Axodox::Storage;

TextureData ReadPfm(const std::filesystem::path& path)
{
  //Create stream
  file_stream stream{ path, file_mode::read };

  //Read header
  auto magic = stream.read_line();
  auto resolution = stream.read_line();
  uint32_t width, height;
  sscanf_s(resolution.c_str(), "%u %u", &width, &height);

  auto bias = stream.read_line();

  //Read depth data
  TextureData result{ width, height, DXGI_FORMAT_R32_FLOAT };
  stream.read(result.Buffer);

  return result;
}

int main()
{
  winrt::init_apartment();
  std::cout << "Hello World!\n";
  
  auto textureData = TextureData::FromBuffer(read_file(L"C:\\dev\\ai\\MiDaS\\input\\test.png"));
  auto depthData = ReadPfm(L"C:\\dev\\ai\\MiDaS\\output\\test-dpt_beit_large_512.pfm");
  
  auto depthBuffer = depthData.Cast<float>();

  Window window{ L"Hello world!" };

  GraphicsDevice device{};
  HwndSwapChain swapChain{ device, *window };

  window.Painting(no_revoke, [&](Window*) {
    swapChain.BackBuffer()->Clear();
    swapChain.Present();
    });

  window.Resized(no_revoke, [&](Window*) {
    swapChain.Resize();
    });

  window.Show();
  window.Run();
}