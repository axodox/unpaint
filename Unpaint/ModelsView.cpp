#include "pch.h"
#include "ModelsView.h"
#include "ModelsView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  ModelsViewModel ModelsView::ViewModel()
  {
    return _viewModel;
  }
}