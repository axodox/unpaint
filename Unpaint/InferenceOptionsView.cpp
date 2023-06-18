#include "pch.h"
#include "InferenceOptionsView.h"
#include "InferenceOptionsView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  InferenceOptionsViewModel InferenceOptionsView::ViewModel()
  {
    return _viewModel;
  }
}
