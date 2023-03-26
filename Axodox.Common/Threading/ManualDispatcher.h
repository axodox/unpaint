#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  class manual_dispatcher
  {
  public:
    std::future<void> invoke_async(std::function<void()> callback);

    void process_pending_invocations();

  private:
    std::queue<std::packaged_task<void()>> _workItems;
    std::mutex _workMutex;
  };
}