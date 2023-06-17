#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "ImportLocalModelDialog.g.h"

namespace winrt::Unpaint::implementation
{
  struct ImportLocalModelDialog : ImportLocalModelDialogT<ImportLocalModelDialog>
  {
    ImportLocalModelViewModel ViewModel();

  private:
    ImportLocalModelViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ImportLocalModelDialog : ImportLocalModelDialogT<ImportLocalModelDialog, implementation::ImportLocalModelDialog>
  {
  };
}
