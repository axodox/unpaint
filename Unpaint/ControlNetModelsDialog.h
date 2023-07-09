#pragma once
#include "ControlNetModelsDialog.g.h"

namespace winrt::Unpaint::implementation
{
  struct ControlNetModelsDialog : ControlNetModelsDialogT<ControlNetModelsDialog>
  {
    ControlNetModelsViewModel ViewModel();

    void OnControlNetModeLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& eventArgs);
    void OnControlNetModeClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& eventArgs);

  private:
    ControlNetModelsViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ControlNetModelsDialog : ControlNetModelsDialogT<ControlNetModelsDialog, implementation::ControlNetModelsDialog>
  {
  };
}
