#include "pch.h"
#include "AsyncOperation.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Threading
{
  async_operation_source::async_operation_source() :
    state_changed(_events)
  { }

  void async_operation_source::update_state(async_operation_state state, float progress, std::string_view message)
  {
    lock_guard lock(_mutex);
    
    _state.state = state;
    if (!isnan(progress)) _state.progress = progress;
    if (!message.empty()) _state.status_message = message;

    _events.raise(state_changed, _state);
  }

  void async_operation_source::update_state(async_operation_state state, std::string_view message)
  {
    lock_guard lock(_mutex);

    _state.state = state;
    if (!message.empty()) _state.status_message = message;

    _events.raise(state_changed, _state);
  }

  void async_operation_source::update_state(float progress, std::string_view message)
  {
    lock_guard lock(_mutex);

    _state.state = async_operation_state::working;
    if (!isnan(progress)) _state.progress = progress;
    if (!message.empty()) _state.status_message = message;

    _events.raise(state_changed, _state);
  }

  void async_operation_source::update_state(std::string_view message)
  {
    lock_guard lock(_mutex);

    _state.state = async_operation_state::working;
    if (!message.empty()) _state.status_message = message;

    _events.raise(state_changed, _state);
  }

  void async_operation_source::cancel()
  {
    _state.is_cancelled = true;
  }

  bool async_operation_source::is_cancelled()
  {
    return _state.is_cancelled;
  }

  void async_operation_source::throw_if_cancelled()
  {
    if (_state.is_cancelled) throw runtime_error("The operation has been cancelled.");
  }

  async_operation_info async_operation_source::state()
  {
    lock_guard lock(_mutex);
    return _state;
  }

  async_operation::async_operation() :
    state_changed(_events),
    _source(nullptr)
  { }

  async_operation_info async_operation::state() const
  {
    return _source ? _source->state() : async_operation_info{};
  }

  void async_operation::cancel()
  {
    if (_source) _source->cancel();
  }

  async_operation::operator bool() const
  {
    return _source != nullptr;
  }
  
  void async_operation::set_source(async_operation_source& source)
  {
    _source = &source;
    _stateChangedSubscription = source.state_changed(event_handler{ this, &async_operation::on_state_changed });
  }

  void async_operation::on_state_changed(const async_operation_info& state)
  {
    _events.raise(state_changed, state);
  }
}