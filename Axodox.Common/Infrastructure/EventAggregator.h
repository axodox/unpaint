#pragma once
#include "Events.h"

namespace Axodox::Infrastructure
{
  class event_aggregator
  {
    struct aggregated_event_base
    {
      virtual ~aggregated_event_base() = default;
    };

    template <typename event_t>
    struct aggregated_event : public aggregated_event_base
    {
      aggregated_event(event_owner& owner) :
        publisher(owner)
      { }
      
      event_publisher<event_t&> publisher;
    };

  private:
    event_owner _eventOwner;
    std::mutex _mutex;
    std::unordered_map<std::type_index, std::unique_ptr<aggregated_event_base>> _events;

    template<typename event_t>
    aggregated_event<event_t>* get_event()
    {
      std::lock_guard lock(_mutex);

      std::type_index index{ typeid(event_t) };
      auto& event = _events[index];
      if (!event)
      {
        event = std::make_unique<aggregated_event<event_t>>(_eventOwner);
      }

      return static_cast<aggregated_event<event_t>*>(event.get());
    }

  public:
    template<typename event_t>
    void raise()
    {
      event_t eventArgs{};
      get_event<event_t>()->publisher.raise(_eventOwner, eventArgs);
    }

    template<typename event_t>
    void raise(event_t& eventArgs)
    {
      get_event<event_t>()->publisher.raise(_eventOwner, eventArgs);
    }

    template<typename event_t>
    event_subscription subscribe(std::function<void(event_t&)>&& handler)
    {
      return get_event<event_t>()->publisher.subscribe(move(handler));
    }
  };
}