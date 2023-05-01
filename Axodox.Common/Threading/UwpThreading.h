#pragma once
#ifdef USE_UWP_UI
#include "pch.h"

namespace Axodox::Threading
{
  void send_or_post(const winrt::Windows::UI::Core::CoreDispatcher& dispatcher, const std::function<void()>& action);
}
#endif