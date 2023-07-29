#include "pch.h"
#include "ControlNetOptionsView.h"
#include "ControlNetOptionsView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  ControlNetOptionsViewModel ControlNetOptionsView::ViewModel()
  {
    return _viewModel;
  }
}
