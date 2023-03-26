// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "UI/Window.h"
#include "Infrastructure/Events.h"
#include "Graphics/Swap Chains/HwndSwapChain.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::UI;

int main()
{
  std::cout << "Hello World!\n";
  
  Window myWindow{ L"Hello world!" };

  GraphicsDevice device{};
  HwndSwapChain swapChain{ device, *myWindow };

  myWindow.Painting(no_revoke, [&](Window*) {
    swapChain.BackBuffer()->Clear();
    swapChain.Present();
    });

  myWindow.Resized(no_revoke, [&](Window*) {
    swapChain.Resize();
    });

  myWindow.Show();
  myWindow.Run();
}