#pragma once
#include "TitleBarHost.g.h"

namespace winrt::Unpaint::implementation
{
  struct TitleBarHost : TitleBarHostT<TitleBarHost>
  {
    bool IsPointerOverTitleBar();

    Windows::UI::Xaml::GridLength TitleBarLeftPadding();
    Windows::UI::Xaml::GridLength TitleBarRightPadding();

    event_token IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler);
    void IsPointerOverTitleBarChanged(event_token const& token) noexcept;

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

    void OnTitleBarLoaded(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& eventArgs);

  private:
    bool _isPointerOverTitleBar = false;
    event<Windows::Foundation::EventHandler<bool>> _isPointerOverTitleBarChanged;
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    Windows::ApplicationModel::Core::CoreApplicationViewTitleBar::LayoutMetricsChanged_revoker _titleBarLayoutMetricsChangedRevoker;
    Windows::UI::Core::CoreWindow::PointerMoved_revoker _pointerMovedRevoker;
    Windows::UI::Core::CoreWindow::PointerExited_revoker _pointerExitedRevoker;

    double _titleBarLeftPadding = 0;
    double _titleBarRightPadding = 0;

    void UpdateTitleBar();
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct TitleBarHost : TitleBarHostT<TitleBarHost, implementation::TitleBarHost>
  {
  };
}
