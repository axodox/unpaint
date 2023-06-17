#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "ImportHuggingFaceModelDialog.g.h"

namespace winrt::Unpaint::implementation
{
  struct ImportHuggingFaceModelDialog : ImportHuggingFaceModelDialogT<ImportHuggingFaceModelDialog>
  {
    ImportHuggingFaceModelViewModel ViewModel();

  private:
    ImportHuggingFaceModelViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ImportHuggingFaceModelDialog : ImportHuggingFaceModelDialogT<ImportHuggingFaceModelDialog, implementation::ImportHuggingFaceModelDialog>
  {
  };
}
