#pragma once
#include "Infrastructure/Win32.h"
#include "Infrastructure/Events.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::UI
{
  class Window
  {
    Infrastructure::event_owner _events;

  public:
    Window(const std::wstring_view title);

    void Show();
    void Hide();

    void Run();

    HWND operator*() const;
    
    Infrastructure::event_publisher<Window*> Resized;
    Infrastructure::event_publisher<Window*> Painting;

  private:
    HWND _window;

    static LRESULT CALLBACK HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  };
}

#endif