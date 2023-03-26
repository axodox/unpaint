#pragma once
#include "pch.h"

namespace Axodox::Threading
{
  typedef std::chrono::duration<uint32_t, std::milli> event_timeout;

  class event_awaiter
  {
  protected:
    std::weak_ptr<winrt::handle> _event;

  public:
    event_awaiter() = default;

    explicit event_awaiter(const std::shared_ptr<winrt::handle>& event) : _event(event)
    { }

    bool wait(event_timeout timeout) const
    {
      auto event = _event.lock();
      if (event)
      {
        return WaitForSingleObject(event->get(), timeout.count()) == WAIT_OBJECT_0;
      }
      else
      {
        return true;
      }
    }
  };

  template<bool CIsManual>
  struct reset_event
  {
  private:
    std::shared_ptr<winrt::handle> _event;

  public:
    reset_event(bool isReady = false)
    {
      _event = std::make_shared<winrt::handle>(CreateEvent(nullptr, CIsManual, isReady, nullptr));
    }

    reset_event(nullptr_t)
    { }

    void set()
    {
      if (_event) SetEvent(_event->get());
    }

    void reset()
    {
      if (_event) ResetEvent(_event->get());
      else throw std::logic_error("Cannot reset a blank event.");
    }

    event_awaiter get_awaiter() const
    {
      return event_awaiter{ _event };
    }

    bool wait(event_timeout timeout) const
    {
      if (_event)
      {
        return WaitForSingleObject(_event->get(), timeout.count()) == WAIT_OBJECT_0;
      }
      else
      {
        return true;
      }
    }

    explicit operator bool() const
    {
      return wait(event_timeout::zero());
    }
  };

  struct manual_reset_event : public reset_event<true>
  {
    using reset_event<true>::reset_event;
  };

  struct auto_reset_event : public reset_event<false>
  {
    using reset_event<false>::reset_event;
  };
}
