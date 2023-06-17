#include "pch.h"
#include "ImportLocalModelDialog.h"
#include "ImportLocalModelDialog.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::Unpaint::implementation
{
  ImportLocalModelViewModel ImportLocalModelDialog::ViewModel()
  {
    return _viewModel;
  }
}
