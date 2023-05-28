#pragma once
#include "pch.h"
#include "SettingControl.g.h"

namespace winrt::Unpaint::implementation
{
  struct SettingControl : SettingControlT<SettingControl>
  {
    SettingControl() = default;

    hstring Icon();
    void Icon(hstring const& value);
    static Windows::UI::Xaml::DependencyProperty IconProperty();

    hstring Title();
    void Title(hstring const& value);
    static Windows::UI::Xaml::DependencyProperty TitleProperty();

    hstring Subtitle();
    void Subtitle(hstring const& value);
    static Windows::UI::Xaml::DependencyProperty SubtitleProperty();

  private:
    static Windows::UI::Xaml::DependencyProperty _iconProperty;
    static Windows::UI::Xaml::DependencyProperty _titleProperty;
    static Windows::UI::Xaml::DependencyProperty _subtitleProperty;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingControl : SettingControlT<SettingControl, implementation::SettingControl>
  {
  };
}
