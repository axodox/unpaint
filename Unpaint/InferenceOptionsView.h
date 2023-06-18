#pragma once
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "InferenceOptionsView.g.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceOptionsView : InferenceOptionsViewT<InferenceOptionsView>
  {
    InferenceOptionsViewModel ViewModel();

  private:
    InferenceOptionsViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceOptionsView : InferenceOptionsViewT<InferenceOptionsView, implementation::InferenceOptionsView>
  {
  };
}
