#pragma once
#include "Infrastructure/Events.h"

namespace Axodox::Threading
{
  enum class async_operation_state
  {
    idle,
    working,
    succeeded,
    failed
  };

  struct async_operation_info
  {
    bool is_cancelled = false;
    async_operation_state state = async_operation_state::idle;
    float progress = NAN;
    std::string status_message;
  };

  class async_operation_source
  {
    Infrastructure::event_owner _events;

  public:
    async_operation_source();

    void update_state(async_operation_state state, float progress = NAN, std::string_view message = "");
    void update_state(async_operation_state state, std::string_view message);
    void update_state(float progress, std::string_view message = "");
    void update_state(std::string_view message);

    void cancel();
    bool is_cancelled();
    void throw_if_cancelled();

    async_operation_info state();
    Infrastructure::event_publisher<const async_operation_info&> state_changed;

  private:
    std::mutex _mutex;
    async_operation_info _state;
  };

  class async_operation
  {
    Infrastructure::event_owner _events;

  public:
    async_operation();

    async_operation_info state() const;
    Infrastructure::event_publisher<const async_operation_info&> state_changed;

    void cancel();

    explicit operator bool() const;

    void set_source(async_operation_source& source);

  private:
    async_operation_source* _source;
    Infrastructure::event_subscription _stateChangedSubscription;

    void on_state_changed(const async_operation_info& state);
  };
}