#include "pch.h"
#include "TitleBarHost.h"
#include "TitleBarHost.g.cpp"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  bool TitleBarHost::IsPointerOverTitleBar()
  {
    return _isPointerOverTitleBar;
  }

  Windows::UI::Xaml::GridLength TitleBarHost::TitleBarLeftPadding()
  {
    return { _titleBarLeftPadding, GridUnitType::Pixel };
  }

  Windows::UI::Xaml::GridLength TitleBarHost::TitleBarRightPadding()
  {
    return { _titleBarRightPadding, GridUnitType::Pixel };
  }

  event_token TitleBarHost::IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler)
  {
    return _isPointerOverTitleBarChanged.add(handler);
  }

  void TitleBarHost::IsPointerOverTitleBarChanged(event_token const& token) noexcept
  {
    _isPointerOverTitleBarChanged.remove(token);
  }

  event_token TitleBarHost::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void TitleBarHost::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }

  void TitleBarHost::OnTitleBarLoaded(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& /*eventArgs*/)
  {
    auto window = Window::Current();
    window.SetTitleBar(sender.try_as<UIElement>());

    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    coreTitleBar.ExtendViewIntoTitleBar(true);

    auto applicationView = ApplicationView::GetForCurrentView();

    auto viewTitleBar = applicationView.TitleBar();
    viewTitleBar.ButtonBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonForegroundColor(Colors::White());
    viewTitleBar.ButtonInactiveBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonInactiveForegroundColor(Colors::White());

    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      UpdateTitleBar();
      });

    UpdateTitleBar();

    auto coreWindow = CoreWindow::GetForCurrentThread();
    _pointerMovedRevoker = coreWindow.PointerMoved(auto_revoke, [=](auto&, PointerEventArgs const& eventArgs) {
      auto verticalPosition = eventArgs.CurrentPoint().Position().Y;
      auto isPointerOverTitleBar = verticalPosition > 0.f && verticalPosition < coreTitleBar.Height();

      if (isPointerOverTitleBar == _isPointerOverTitleBar) return;

      _isPointerOverTitleBar = isPointerOverTitleBar;
      _isPointerOverTitleBarChanged(*this, isPointerOverTitleBar);
      });

    _pointerExitedRevoker = coreWindow.PointerExited(auto_revoke, [=](auto&, auto&) {
      if (_isPointerOverTitleBar) return;

      _isPointerOverTitleBar = true;
      _isPointerOverTitleBarChanged(*this, true);
      });
  }

  void TitleBarHost::UpdateTitleBar()
  {
    auto titleBar = CoreApplication::GetCurrentView().TitleBar();

    auto height = titleBar.Height();
    if (height == 0.f) height = 32.f;

    Height(height);
    _titleBarLeftPadding = titleBar.SystemOverlayLeftInset();
    _titleBarRightPadding = titleBar.SystemOverlayRightInset();

    _propertyChanged(*this, PropertyChangedEventArgs(L"TitleBarLeftPadding"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"TitleBarRightPadding"));
  }
}
