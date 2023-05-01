#include "pch.h"
#include "DownloadModelDialog.h"
#include "DownloadModelDialog.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  DownloadModelDialog::DownloadModelDialog(hstring const& modelId) :
    _viewModel(modelId)
  { }

  DownloadModelViewModel DownloadModelDialog::ViewModel()
  {
    return _viewModel;
  }
}
