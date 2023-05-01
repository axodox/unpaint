#pragma once
#include "DownloadModelDialog.g.h"

namespace winrt::Unpaint::implementation
{
  struct DownloadModelDialog : DownloadModelDialogT<DownloadModelDialog>
  {
    DownloadModelDialog(hstring const& modelId);

    DownloadModelViewModel ViewModel();

  private:
    DownloadModelViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct DownloadModelDialog : DownloadModelDialogT<DownloadModelDialog, implementation::DownloadModelDialog>
  {
  };
}
