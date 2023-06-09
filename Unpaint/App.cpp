#include "pch.h"
#include "App.h"
#include "Infrastructure/WinRtDependencies.h"
#include "Threading/Parallel.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Unpaint;
using namespace winrt::Unpaint::implementation;

/// <summary>
/// Creates the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App() :
  _frame(nullptr)
{
  set_thread_name(L"* ui");
  dependencies.add<INavigationService>(*this);
  _unpaintOptions = dependencies.resolve<UnpaintOptions>();
  _modelRepository = dependencies.resolve<ModelRepository>();

  Suspending({ this, &App::OnSuspending });

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
      if (IsDebuggerPresent())
      {
        auto errorMessage = e.Message();
        __debugbreak();
      }
    });
#endif
}

void App::Activate(Windows::ApplicationModel::Activation::IActivatedEventArgs eventArgs)
{
  auto window = Window::Current();  

  _frame = window.Content().try_as<Frame>();

  auto launchActivatedEventArgs = eventArgs.try_as<LaunchActivatedEventArgs>();
  if (_frame == nullptr)
  {
    _frame = Frame();
    _frame.SetValue(BackdropMaterial::ApplyToRootOrPageBackgroundProperty(), box_value(true));
    _frame.NavigationFailed({ this, &App::OnNavigationFailed });
    window.Content(_frame);
    NavigateToView(xaml_typename<Unpaint::InferenceView>());
  }

  auto protocolActivatedEventArgs = eventArgs.try_as<ProtocolActivatedEventArgs>();
  if (protocolActivatedEventArgs)
  {
    NavigateToView(xaml_typename<Unpaint::InferenceView>());

    auto inferenceView = _frame.Content().try_as<Unpaint::InferenceView>();
    if (inferenceView) inferenceView.ViewModel().OpenUri(protocolActivatedEventArgs.Uri());
  }

  auto coreWindow = CoreWindow::GetForCurrentThread();
  auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();

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

  Window::Current().Activate();
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const& eventArgs)
{
  Activate(eventArgs);
}

void App::OnActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs const& eventArgs)
{
  Activate(eventArgs);
}

/// <summary>
/// Invoked when application execution is being suspended.  Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending([[maybe_unused]] IInspectable const& sender, [[maybe_unused]] SuspendingEventArgs const& e)
{
  // Save application state and stop any background activity
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(IInspectable const&, NavigationFailedEventArgs const& e)
{
  throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

void App::NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType)
{
  auto window = Window::Current();
  window.SetTitleBar(nullptr);

  if (!_unpaintOptions->HasShownShowcaseView())
  {
    viewType = xaml_typename<Unpaint::ShowcaseView>();
  }
  else if (!_unpaintOptions->HasShownWelcomeView())
  {
    viewType = xaml_typename<Unpaint::WelcomeView>();
  }
  else if (viewType == xaml_typename<Unpaint::InferenceView>() && _modelRepository->Models().empty())
  {
    viewType = xaml_typename<Unpaint::ModelsView>();
  }

  if (_frame.SourcePageType().Name != viewType.Name)
  {
    _frame.Navigate(viewType);
  }
}

bool App::IsPointerOverTitleBar()
{
  return _isPointerOverTitleBar;
}

event_token App::IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler)
{
  return _isPointerOverTitleBarChanged.add(handler);
}

void App::IsPointerOverTitleBarChanged(event_token const& token) noexcept
{
  _isPointerOverTitleBarChanged.remove(token);
}