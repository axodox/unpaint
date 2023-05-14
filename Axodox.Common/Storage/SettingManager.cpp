#include "pch.h"
#include "SettingManager.h"

using namespace winrt;
using namespace winrt::Windows::Storage;

namespace Axodox::Storage
{
  SettingManager::SettingManager() :
    _settings(ApplicationData::Current().LocalSettings().Values())
  { }

  bool SettingManager::HasSetting(std::string_view key)
  {
    return _settings.HasKey(to_hstring(key));
  }

  void SettingManager::RemoveSetting(std::string_view key)
  {
    _settings.Remove(to_hstring(key));
  }
}