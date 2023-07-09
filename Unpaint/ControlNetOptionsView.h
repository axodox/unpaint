#pragma once
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "ControlNetOptionsView.g.h"

namespace winrt::Unpaint::implementation
{
  struct ControlNetOptionsView : ControlNetOptionsViewT<ControlNetOptionsView>
  {
    ControlNetOptionsViewModel ViewModel();

  private:
    ControlNetOptionsViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ControlNetOptionsView : ControlNetOptionsViewT<ControlNetOptionsView, implementation::ControlNetOptionsView>
  {
  };
}
