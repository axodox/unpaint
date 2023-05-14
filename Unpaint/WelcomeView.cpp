#include "pch.h"
#include "WelcomeView.h"
#include "WelcomeView.g.cpp"
#include "Infrastructure/WinRtDependencies.h"
#include "Storage/SettingManager.h"
#include "UnpaintSettings.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;

namespace winrt::Unpaint::implementation
{
  void WelcomeView::OnContinueClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
  {
    dependencies.resolve<SettingManager>()->StoreSetting(Settings::UserInterface::HasShownWelcomeView, true);
    dependencies.resolve<INavigationService>().NavigateToView(xaml_typename<InferenceView>());
  }
}