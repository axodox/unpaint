#pragma once
#include "Threading/Events.h"

namespace Axodox::Infrastructure
{
  template <typename... TArgs>
  class event_handler : public std::function<void(TArgs...)>
  {
  public:
    template<typename TInstance, typename TReturn = void>
    event_handler(TInstance* instance, TReturn(TInstance::* handler)(TArgs...)) :
      std::function<void(TArgs...)>([=](TArgs&&... args) { (instance->*handler)(std::forward<TArgs>(args)...); })
    { }

    using std::function<void(TArgs...)>::function;
  };

  class event_handler_collection_base
  {
  public:
    typedef uint32_t token_t;

    virtual bool remove(token_t token) noexcept = 0;

    virtual ~event_handler_collection_base() noexcept = default;
  };

  class [[nodiscard]] event_subscription
  {
  public:
    event_subscription() = default;

    event_subscription& operator=(event_subscription&& other) noexcept
    {
      _handlers = std::move(other._handlers);
      _token = other._token;

      other._token = {};
      return *this;
    }

    event_subscription(event_subscription&& other) noexcept
    {
      *this = std::move(other);
    }

    event_subscription(const event_subscription&) = delete;
    event_subscription& operator=(const event_subscription&) = delete;

    event_subscription(const std::shared_ptr<event_handler_collection_base>& eventHandlerCollection, event_handler_collection_base::token_t token) noexcept :
      _handlers(eventHandlerCollection),
      _token(token)
    { }

    ~event_subscription() noexcept
    {
      auto eventHandlerCollection = _handlers.lock();
      if (eventHandlerCollection)
      {
        eventHandlerCollection->remove(_token);
      }
    }

    bool is_valid() const noexcept
    {
      return _handlers.lock() != nullptr;
    }

  private:
    std::weak_ptr<event_handler_collection_base> _handlers;
    event_handler_collection_base::token_t _token;
  };

  template<typename... TArgs>
  class event_handler_collection : public event_handler_collection_base
  {
  public:
    typedef std::function<void(TArgs...)> handler_t;

    token_t add(handler_t&& handler) noexcept
    {
      std::unique_lock lock(_mutex);

      auto token = _nextToken++;
      _handlers[token] = std::move(handler);
      return token;
    }

    virtual bool remove(token_t token) noexcept override
    {
      std::unique_lock lock(_mutex);
      return _handlers.erase(token);
    }

    void invoke(TArgs... args)
    {
      std::shared_lock lock(_mutex);
      for (auto& [token, handler] : _handlers)
      {
        handler(std::forward<TArgs>(args)...);
      }
    }

  private:
    std::shared_mutex _mutex;
    std::unordered_map<token_t, handler_t> _handlers;
    token_t _nextToken = {};
  };

  struct no_revoke_t {};
  inline constexpr no_revoke_t no_revoke{};

  template<typename... TArgs>
  class event_publisher;

  class event_owner
  {
  public:
    event_owner() noexcept :
      _id(_nextId++)
    { }

    event_owner& operator=(event_owner&& other) noexcept
    {
      _id = other._id;
      other._id = size_t{};
      return *this;
    }

    event_owner(event_owner&& other) noexcept
    {
      *this = std::move(other);
    }

    event_owner(const event_owner&) = delete;
    event_owner& operator=(const event_owner&) = delete;

    template<typename... TArgs, typename TEvent = event_publisher<TArgs...>>
    void raise(TEvent& event, TArgs... args) const;

    bool is_valid() const noexcept
    {
      return _id != size_t{};
    }

    operator size_t() const noexcept
    {
      return _id;
    }

  private:
    static inline std::atomic_size_t _nextId = 1;
    size_t _id;
  };

  template<typename... TArgs>
  class event_publisher
  {
    friend class event_owner;

  private:
    size_t _owner_id;
    std::shared_ptr<event_handler_collection<TArgs...>> _handlers;

  public:
    event_publisher(event_publisher&&) = default;
    event_publisher& operator=(event_publisher&&) = default;

    event_publisher(const event_publisher&) = delete;
    event_publisher& operator=(const event_publisher&) = delete;

    event_publisher(const event_owner& owner) noexcept :
      _owner_id(owner),
      _handlers(std::make_shared<event_handler_collection<TArgs...>>())
    { }

    event_subscription subscribe(std::function<void(TArgs...)>&& handler) noexcept
    {
      return { _handlers, _handlers->add(std::move(handler)) };
    }

    void subscribe(no_revoke_t, std::function<void(TArgs...)>&& handler) noexcept
    {
      _handlers->add(std::move(handler));
    }

    event_subscription operator()(std::function<void(TArgs...)>&& handler) noexcept
    {
      return subscribe(std::move(handler));
    }

    void operator()(no_revoke_t, std::function<void(TArgs...)>&& handler) noexcept
    {
      subscribe(no_revoke, std::move(handler));
    }

    void raise(const event_owner& owner, TArgs... args)
    {
      if (owner != _owner_id)
      {
        throw std::logic_error("The specified event owner does not own this event.");
      }

      _handlers->invoke(std::forward<TArgs>(args)...);
    }

    std::tuple<TArgs...> wait(Threading::event_timeout timeout)
    {
      if (timeout == Threading::event_timeout::zero()) return {};

      std::tuple<TArgs...> result{};

      Threading::manual_reset_event awaiter;
      auto subscription = subscribe([&](TArgs... args) -> void {
        result = { args... };
        awaiter.set();
        });

      awaiter.wait(timeout);
      return result;
    }

    ~event_publisher() noexcept
    {
      _handlers.reset();
    }
  };

  template<typename... TArgs, typename TEvent>
  void event_owner::raise(TEvent& event, TArgs... args) const
  {
    event.raise(*this, std::forward<TArgs>(args)...);
  }
}