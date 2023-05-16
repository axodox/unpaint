#pragma once
#include "Storage/SettingManager.h"

namespace winrt::Unpaint
{
  class UnpaintOptions
  {
  public:
    UnpaintOptions();

    bool HasShownWelcomeView() const;
    void HasShownWelcomeView(bool value);

    bool IsSafeModeEnabled() const;
    void IsSafeModeEnabled(bool value);

    bool IsDenoiserPinned() const;
    void IsDenoiserPinned(bool value);

  private:
    static const char* _hasShownWelcomeViewKey;
    static const char* _isSafeModeEnabledKey;
    static const char* _isDenoiserPinnedKey;

    std::shared_ptr<Axodox::Storage::SettingManager> _settingManager;
  };
}