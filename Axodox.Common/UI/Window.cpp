#include "pch.h"
#include "Window.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Axodox::UI
{
  Window::Window(const std::wstring_view title) :
    Resized(_events),
    Painting(_events)
  {
    //Register window class
    WNDCLASSW windowClass;
    {
      zero_memory(windowClass);

      windowClass.lpfnWndProc = &Window::HandleMessage;
      windowClass.hInstance = GetModuleHandle(nullptr);
      windowClass.lpszClassName = L"axodox-window";

      RegisterClassW(&windowClass);
    }

    //Create window
    _window = CreateWindowExW(
      0,
      windowClass.lpszClassName,
      title.data(),
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
      nullptr,
      nullptr,
      windowClass.hInstance,
      this);
  }

  void Window::Show()
  {
    ShowWindow(_window, SW_NORMAL);
  }

  void Window::Hide()
  {
    check_bool(ShowWindow(_window, SW_HIDE));
  }

  void Window::Run()
  {
    MSG message = { };
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
  }

  HWND Window::operator*() const
  {
    return _window;
  }

  LRESULT Window::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
  {
    auto that = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
      SetWindowLongPtrW(window, GWLP_USERDATA, LONG_PTR(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
      break;
    case WM_SIZE:
      that->_events.raise(that->Resized, that);
      break;
    case WM_PAINT:
      that->_events.raise(that->Painting, that);
      break;
    case WM_CLOSE:
      DestroyWindow(window);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
  }
}