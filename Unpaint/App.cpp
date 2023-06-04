#include "pch.h"
#include "App.h"
#include "MainView.h"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Unpaint;
using namespace winrt::Unpaint::implementation;

/// <summary>
/// Creates the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
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
  Frame rootFrame{ nullptr };
  auto content = Window::Current().Content();
  if (content)
  {
    rootFrame = content.try_as<Frame>();
  }

  auto launchActivatedEventArgs = eventArgs.try_as<LaunchActivatedEventArgs>();
  if (rootFrame == nullptr)
  {
    rootFrame = Frame();
    rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });
    Window::Current().Content(rootFrame);    
    rootFrame.Navigate(xaml_typename<Unpaint::MainView>(), eventArgs);
  }

  auto protocolActivatedEventArgs = eventArgs.try_as<ProtocolActivatedEventArgs>();
  if (protocolActivatedEventArgs)
  {
    auto mainView = rootFrame.Content().as<MainView>();
    mainView->NavigateToView(xaml_typename<InferenceView>());

    auto inferenceView = mainView->ContentFrame().Content().try_as<InferenceView>();
    if (inferenceView) inferenceView.ViewModel().OpenUri(protocolActivatedEventArgs.Uri());
  }

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