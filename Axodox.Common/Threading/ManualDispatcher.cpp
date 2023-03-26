#include "pch.h"
#include "ManualDispatcher.h"

using namespace std;

namespace Axodox::Threading
{
  future<void> manual_dispatcher::invoke_async(function<void()> callback)
  {
    auto task = packaged_task<void()>(callback);
    auto future = task.get_future();
    {
      lock_guard lock(_workMutex);
      _workItems.push(move(task));
    }
    return future;
  }

  void manual_dispatcher::process_pending_invocations()
  {
    packaged_task<void()> workItem;
    while (true)
    {
      {
        unique_lock lock(_workMutex);
        if (_workItems.empty()) return;

        workItem = move(_workItems.front());
        _workItems.pop();
      }

      workItem();
    }
  }
}
