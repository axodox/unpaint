#include "pch.h"
#include "ControlNetModelsDialog.h"
#include "ControlNetModelsDialog.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;

namespace winrt::Unpaint::implementation
{
  ControlNetModelsViewModel ControlNetModelsDialog::ViewModel()
  {
    return _viewModel;
  }

  void ControlNetModelsDialog::OnControlNetModeLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*eventArgs*/)
  {
    auto checkBox = sender.as<CheckBox>();
    auto tag = unbox_value_or<hstring>(checkBox.Tag(), L"");
    
    checkBox.IsChecked(_viewModel.IsSelected(tag));
  }

  void ControlNetModelsDialog::OnControlNetModeClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*eventArgs*/)
  {
    auto checkBox = sender.as<CheckBox>();
    auto tag = unbox_value_or<hstring>(checkBox.Tag(), L"");

    _viewModel.IsSelected(tag, checkBox.IsChecked().Value());
  }
}
