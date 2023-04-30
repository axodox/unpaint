#pragma once
#include "Traits.h"

namespace Axodox::Infrastructure
{
  class dependency_container;

  struct expects_dependency_container_test
  {
    typedef dependency_container* arg_t;

    template<typename T>
    static std::true_type test(decltype(T(arg_t()))*);

    template<typename T>
    static std::false_type test(...);
  };

  template<class T>
  struct expects_dependency_container : decltype(expects_dependency_container_test::test<T>(nullptr))
  {};

  template<typename T, typename TEnable = void>
  struct dependency_pointer_type
  {
    using type = std::shared_ptr<T>;

    inline static std::shared_ptr<T> default_value = nullptr;

    static std::function<std::shared_ptr<T>()> get_factory(dependency_container* container)
    {
      if constexpr (supports_new<T>::value)
      {
        return [] { return std::make_shared<T>(); };
      }
      else if constexpr (expects_dependency_container<T>::value)
      {
        return [=] { return std::make_shared<T>(container); };
      }
      else
      {
        return [=] { return std::shared_ptr<T>(nullptr); };
      }
    }

    template<typename... TArgs>
    static std::shared_ptr<T> make(TArgs&&... args)
    {
      return std::make_shared<T>(std::forward<TArgs>(args)...);
    }

    template<typename U>
    static std::shared_ptr<U> cast(const std::shared_ptr<T>& value)
    {
      return std::static_pointer_cast<U>(value);
    }
  };

  template<typename T>
  using dependency_ptr = typename dependency_pointer_type<T>::type;

  enum class dependency_lifetime
  {
    singleton,
    transient
  };

  struct dependency_registration_base
  {
    virtual void clone_into(dependency_container* container) = 0;

    virtual ~dependency_registration_base() = default;
  };

  template<typename T>
  class dependency_registration : public dependency_registration_base
  {
  private:
    dependency_container* _owner;
    dependency_lifetime _lifetime;
    dependency_ptr<T> _value = dependency_pointer_type<T>::default_value;
    std::function<dependency_ptr<T>()> _factory;
    std::mutex _mutex;

    dependency_registration() = default;

  public:
    dependency_registration(dependency_container* owner, dependency_lifetime lifetime) :
      _owner(owner),
      _lifetime(lifetime)
    {
      _factory = dependency_pointer_type<T>::get_factory(owner);
    }

    dependency_registration(dependency_container* owner, dependency_lifetime lifetime, std::function<dependency_ptr<T>()>&& factory) :
      _owner(owner),
      _lifetime(lifetime),
      _factory{ std::move(factory) }
    { }

    dependency_ptr<T> try_get()
    {
      switch (_lifetime)
      {
      case dependency_lifetime::singleton:
      {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_value)
        {
          _value = _factory();
        }

        return _value;
      }
      case dependency_lifetime::transient:
      {
        return _factory();
      }
      default:
        return dependency_pointer_type<T>::default_value;
      }
    }

    dependency_ptr<T> get()
    {
      auto result = try_get();
      if (bool(result))
      {
        return result;
      }
      else
      {
        throw std::logic_error("Failed to construct type.");
      }
    }

    virtual void clone_into(dependency_container* container) override;
  };

  struct dependency_container_data
  {
    std::unordered_map<std::type_index, std::unique_ptr<dependency_registration_base>> registrations;
    std::recursive_mutex mutex;
  };

  class dependency_container_ref
  {
  private:
    std::weak_ptr<dependency_container_data> _data;

  public:
    dependency_container_ref()
    { }

    dependency_container_ref(const std::weak_ptr<dependency_container_data>& data) :
      _data(data)
    { }

    std::unique_ptr<dependency_container> try_lock() const;
  };

  class dependency_container
  {
    friend class dependency_container_ref;

    template<typename T> friend class dependency_registration;

  private:
    std::shared_ptr<dependency_container_data> _data;

    template<typename T>
    dependency_registration<T>* get_registration()
    {
      std::type_index index{ typeid(T) };

      dependency_registration<T>* registration;
      {
        std::lock_guard<std::recursive_mutex> lock(_data->mutex);

        auto& item = _data->registrations[index];
        if (!item)
        {
          item = std::make_unique<dependency_registration<T>>(this, dependency_lifetime::singleton);
        }

        registration = static_cast<dependency_registration<T>*>(item.get());
      }

      return registration;
    }

    explicit dependency_container(const std::shared_ptr<dependency_container_data>& data) :
      _data(data)
    {
      assert(data != nullptr);
    }

  public:
    dependency_container(const dependency_container&) = delete;
    dependency_container& operator=(const dependency_container&) = delete;

    dependency_container(dependency_container&&) = delete;
    dependency_container& operator=(dependency_container&&) = delete;

    dependency_container() :
      _data(std::make_shared<dependency_container_data>())
    { }

    dependency_container_ref get_ref() const
    {
      return { _data };
    }

    template<typename T>
    dependency_ptr<T> resolve()
    {
      auto registration = get_registration<T>();
      return registration->get();
    }

    template<typename T, typename U>
    dependency_ptr<U> resolve_as()
    {
      add<T, U>();
      auto registration = get_registration<T>();
      return dependency_pointer_type<T>::template cast<U>(registration->get());
    }

    template<typename T>
    dependency_ptr<T> try_resolve()
    {
      auto registration = get_registration<T>();
      return registration->try_get();
    }

    template<typename T>
    bool add(dependency_lifetime lifetime)
    {
      std::type_index index{ typeid(T) };

      std::lock_guard<std::recursive_mutex> lock(_data->mutex);
      return _data->registrations.emplace(index, std::make_unique<dependency_registration<T>>(this, lifetime)).second;
    }

    template<typename T>
    bool add(dependency_lifetime lifetime, std::function<dependency_ptr<T>()>&& factory)
    {
      std::type_index index{ typeid(T) };

      std::lock_guard<std::recursive_mutex> lock(_data->mutex);
      return _data->registrations.emplace(index, std::make_unique<dependency_registration<T>>(this, lifetime, std::move(factory))).second;
    }

    template<typename T>
    bool add(std::function<dependency_ptr<T>()>&& factory)
    {
      return add<T>(dependency_lifetime::singleton, move(factory));
    }

    template<typename T>
    bool add(dependency_lifetime lifetime, std::function<dependency_ptr<T>(dependency_container*)>&& factory)
    {
      std::type_index index{ typeid(T) };

      std::lock_guard<std::recursive_mutex> lock(_data->mutex);
      return _data->registrations.emplace(index, std::make_unique<dependency_registration<T>>(this, lifetime, [=] { return factory(this); })).second;
    }

    template<typename T>
    bool add(std::function<dependency_ptr<T>(dependency_container*)>&& factory)
    {
      return add<T>(dependency_lifetime::singleton, move(factory));
    }

    template<typename T>
    bool add(const dependency_ptr<T>& instance)
    {
      return add<T>(dependency_lifetime::singleton, std::function<dependency_ptr<T>()>([=] { return instance; }));
    }

    template<typename T, typename U>
    bool add()
    {
      return add<T>(dependency_lifetime::singleton, std::function<dependency_ptr<T>()>([&] { return resolve<U>(); }));
    }

    template<typename T, typename... TArgs>
    dependency_ptr<T> create(TArgs&&... args)
    {
      auto result = dependency_pointer_type<T>::make(std::forward<TArgs>(args)...);
      if (add<T>(result))
      {
        return result;
      }
      else
      {
        return resolve<T>();
      }
    }

    std::unique_ptr<dependency_container> create_child()
    {
      std::lock_guard<std::recursive_mutex> lock(_data->mutex);

      auto child = std::make_unique<dependency_container>();
      for (auto& [key, registration] : _data->registrations)
      {
        registration->clone_into(child.get());
      }
      return child;
    }
  };

  inline std::unique_ptr<dependency_container> dependency_container_ref::try_lock() const
  {
    auto data = _data.lock();
    if (data)
    {
      return std::unique_ptr<dependency_container>(new dependency_container(data));
    }
    else
    {
      return nullptr;
    }
  }

  template<typename T>
  void dependency_registration<T>::clone_into(dependency_container* container)
  {
    std::type_index index{ typeid(T) };

    auto clone = std::unique_ptr<dependency_registration<T>>(new dependency_registration<T>());
    clone->_factory = _factory;
    clone->_lifetime = _lifetime;
    clone->_value = _value;
    clone->_owner = container;

    container->_data->registrations[index] = std::move(clone);
  }

#ifdef USE_GLOBAL_DEPENDENCIES
  extern dependency_container dependencies;
#endif // USE_GLOBAL_DEPENDENCIES
}