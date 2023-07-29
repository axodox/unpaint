#pragma once
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "PromptEditorView.g.h"

namespace winrt::Unpaint::implementation
{
  struct PromptEditorView : PromptEditorViewT<PromptEditorView>
  {
    PromptEditorViewModel ViewModel();

  private:
    PromptEditorViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct PromptEditorView : PromptEditorViewT<PromptEditorView, implementation::PromptEditorView>
  {
  };
}
