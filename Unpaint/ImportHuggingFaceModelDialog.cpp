#include "pch.h"
#include "ImportHuggingFaceModelDialog.h"
#include "ImportHuggingFaceModelDialog.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  ImportHuggingFaceModelViewModel ImportHuggingFaceModelDialog::ViewModel()
  {
    return _viewModel;
  }
}
