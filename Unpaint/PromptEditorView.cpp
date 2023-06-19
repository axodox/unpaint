#include "pch.h"
#include "PromptEditorView.h"
#include "PromptEditorView.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  PromptEditorViewModel PromptEditorView::ViewModel()
  {
    return _viewModel;
  }
}
