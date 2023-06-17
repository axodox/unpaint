#include "pch.h"
#include "SettingControl.h"
#include "SettingControl.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;

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

  Windows::UI::Xaml::Controls::Orientation SettingControl::Orientation()
  {
    return unbox_value_or(GetValue(_orientationProperty), Orientation::Horizontal);
  }

  void SettingControl::Orientation(Windows::UI::Xaml::Controls::Orientation value)
  {
    SetValue(_orientationProperty, box_value(value));
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::OrientationProperty()
  {
    return _orientationProperty;
  }

  bool SettingControl::HasSubtitle()
  {
    return !Subtitle().empty();
  }

  int32_t SettingControl::ContentRow()
  {
    return Orientation() == Orientation::Horizontal ? 0 : 1;
  }

  int32_t SettingControl::ContentColumn()
  {
    return Orientation() == Orientation::Horizontal ? 2 : 0;
  }

  event_token SettingControl::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void SettingControl::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }

  void SettingControl::OnSubtitlePropertyChanged(Windows::UI::Xaml::DependencyObject const& sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const& /*eventArgs*/)
  {
    auto that = sender.as<SettingControl>();
    that->_propertyChanged(*that, PropertyChangedEventArgs(L"HasSubtitle"));
  }

  void SettingControl::OnOrientationPropertyChanged(Windows::UI::Xaml::DependencyObject const& sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const& /*eventArgs*/)
  {
    auto that = sender.as<SettingControl>();
    that->_propertyChanged(*that, PropertyChangedEventArgs(L"ContentRow"));
    that->_propertyChanged(*that, PropertyChangedEventArgs(L"ContentColumn"));
  }

  Windows::UI::Xaml::DependencyProperty SettingControl::_iconProperty = DependencyProperty::Register(L"Icon", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L"")));
  Windows::UI::Xaml::DependencyProperty SettingControl::_titleProperty = DependencyProperty::Register(L"Title", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L"")));
  Windows::UI::Xaml::DependencyProperty SettingControl::_subtitleProperty = DependencyProperty::Register(L"Subtitle", xaml_typename<hstring>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(L""), &OnSubtitlePropertyChanged));
  Windows::UI::Xaml::DependencyProperty SettingControl::_orientationProperty = DependencyProperty::Register(L"Orientation", xaml_typename<Controls::Orientation>(), xaml_typename<Unpaint::SettingControl>(), PropertyMetadata(box_value(Orientation::Horizontal), &OnOrientationPropertyChanged));
}
