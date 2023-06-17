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

    Windows::UI::Xaml::Controls::Orientation Orientation();
    void Orientation(Windows::UI::Xaml::Controls::Orientation value);
    static Windows::UI::Xaml::DependencyProperty OrientationProperty();

    bool HasSubtitle();
    int32_t ContentRow();
    int32_t ContentColumn();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    static Windows::UI::Xaml::DependencyProperty _iconProperty;
    static Windows::UI::Xaml::DependencyProperty _titleProperty;
    static Windows::UI::Xaml::DependencyProperty _subtitleProperty;
    static Windows::UI::Xaml::DependencyProperty _orientationProperty;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    static void OnSubtitlePropertyChanged(Windows::UI::Xaml::DependencyObject const& sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const& eventArgs);
    static void OnOrientationPropertyChanged(Windows::UI::Xaml::DependencyObject const& sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const& eventArgs);
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct SettingControl : SettingControlT<SettingControl, implementation::SettingControl>
  {
  };
}
