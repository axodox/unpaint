#include "pch.h"
#include "ShowcaseView.h"
#include "ShowcaseView.g.cpp"
#include "Infrastructure/DependencyContainer.h"
#include "UnpaintState.h"

using namespace Axodox::Infrastructure;
using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  ShowcaseView::ShowcaseView()
  {
    dependencies.resolve<UnpaintState>()->HasShownShowcaseView = true;
  }

  ShowcaseViewModel ShowcaseView::ViewModel()
  {
    return _viewModel;
  }
}
