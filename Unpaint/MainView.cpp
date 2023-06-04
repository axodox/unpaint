#include "pch.h"
#include "MainView.h"
#include "MainView.g.cpp"
#include "Infrastructure/WinRtDependencies.h"
#include "Threading/Parallel.h"
#include "UnpaintOptions.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::ViewManagement; 
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  MainView::MainView() :
    _unpaintOptions(dependencies.resolve<UnpaintOptions>()),
    _modelRepository(dependencies.resolve<ModelRepository>()),
    _isPointerOverTitleBar(false)
  {
    set_thread_name(L"* ui");

    //Initialize main view
    InitializeComponent();
    InitializeTitleBar();

    dependencies.add<INavigationService>(*this);
    NavigateToView(xaml_typename<InferenceView>());
  }

  void MainView::NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType)
  {
    if (!_unpaintOptions->HasShownShowcaseView())
    {
      viewType = xaml_typename<ShowcaseView>();
    }
    else if (!_unpaintOptions->HasShownWelcomeView())
    {
      viewType = xaml_typename<WelcomeView>();
    }
    else if (viewType == xaml_typename<InferenceView>() && _modelRepository->Models().empty())
    {
      viewType = xaml_typename<ModelsView>();
    }
    
    if (ContentFrame().SourcePageType().Name != viewType.Name)
    {
      ContentFrame().Navigate(viewType);
    }
  }

  bool MainView::IsPointerOverTitleBar()
  {
    return _isPointerOverTitleBar;
  }

  event_token MainView::IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler)
  {
    return _isPointerOverTitleBarChanged.add(handler);
  }

  void MainView::IsPointerOverTitleBarChanged(event_token const& token) noexcept
  {
    _isPointerOverTitleBarChanged.remove(token);
  }

  void MainView::InitializeTitleBar()
  {
    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    coreTitleBar.ExtendViewIntoTitleBar(true);

    auto applicationView = ApplicationView::GetForCurrentView();
    applicationView.SetPreferredMinSize({ 1400, 840 });

    auto viewTitleBar = applicationView.TitleBar();
    viewTitleBar.ButtonBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonForegroundColor(Colors::White());
    viewTitleBar.ButtonInactiveBackgroundColor(Colors::Transparent());
    viewTitleBar.ButtonInactiveForegroundColor(Colors::White());

    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      UpdateTitleBar();
      });

    UpdateTitleBar();

    auto window = Window::Current();
    window.SetTitleBar(TitleBar());
    
    auto coreWindow = CoreWindow::GetForCurrentThread();
    coreWindow.PointerMoved([=](auto&, PointerEventArgs const& eventArgs) {
      auto verticalPosition = eventArgs.CurrentPoint().Position().Y;
      auto isPointerOverTitleBar = verticalPosition > 0.f && verticalPosition < coreTitleBar.Height();

      if (isPointerOverTitleBar == _isPointerOverTitleBar) return;

      _isPointerOverTitleBar = isPointerOverTitleBar;
      _isPointerOverTitleBarChanged(*this, isPointerOverTitleBar);
      });
    coreWindow.PointerExited([=](auto&, auto&) {
      if (_isPointerOverTitleBar) return;

      _isPointerOverTitleBar = true;
      _isPointerOverTitleBarChanged(*this, true);
      });
  }
  
  void MainView::UpdateTitleBar()
  {
    auto titleBar = CoreApplication::GetCurrentView().TitleBar();

    TitleBar().Height(titleBar.Height());
    TitleBarLeftPaddingColumn().Width({ titleBar.SystemOverlayLeftInset() });
    TitleBarRightPaddingColumn().Width({ titleBar.SystemOverlayRightInset() });
  }
}
