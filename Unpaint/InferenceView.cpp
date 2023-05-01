#include "pch.h"
#include "InferenceView.h"
#include "InferenceView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  InferenceViewModel InferenceView::ViewModel()
  {
    return _viewModel;
  }
}
