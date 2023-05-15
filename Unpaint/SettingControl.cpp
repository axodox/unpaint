#include "pch.h"
#include "SettingControl.h"
#include "SettingControl.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  hstring SettingControl::Icon()
  {
    return unbox_value_or(GetValue(_iconProperty), L"");
  }

  void SettingControl::Icon(hstring const& value)
  {
    SetValue(_iconProperty, box_value(value));
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::IconProperty()
  {
    return _iconProperty;
  }

  hstring SettingControl::Title()
  {
    return unbox_value_or(GetValue(_titleProperty), L"");
  }

  void SettingControl::Title(hstring const& value)
  {
    SetValue(_titleProperty, box_value(value));
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::TitleProperty()
  {
    return _titleProperty;
  }

  hstring SettingControl::Subtitle()
  {
    return unbox_value_or(GetValue(_subtitleProperty), L"");
  }

  void SettingControl::Subtitle(hstring const& value)
  {
    SetValue(_subtitleProperty, box_value(value));
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::SubtitleProperty()
  {
    return _subtitleProperty;
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::_iconProperty = DependencyProperty::Register(L"Icon", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L"")));
  Windows::UI::Xaml::DependencyProperty SettingControl::_titleProperty = DependencyProperty::Register(L"Title", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L"")));
  Windows::UI::Xaml::DependencyProperty SettingControl::_subtitleProperty = DependencyProperty::Register(L"Subtitle", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L"")));
}
