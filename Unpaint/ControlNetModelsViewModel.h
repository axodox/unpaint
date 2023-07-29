#pragma once
#include "ControlNetModelsViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ControlNetModelsViewModel : ControlNetModelsViewModelT<ControlNetModelsViewModel>
  {
    ControlNetModelsViewModel();

    Windows::Foundation::Collections::IObservableVector<ControlNetModeViewModel> Modes();

    bool IsSelected(hstring const& modeId);
    void IsSelected(hstring const& modeId, bool value);

    Windows::Foundation::Collections::IVector<hstring> SelectedModes();
    void SelectedModes(Windows::Foundation::Collections::IVector<hstring> const& value);

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    Windows::Foundation::Collections::IObservableVector<ControlNetModeViewModel> _modes;
    std::set<hstring> _selectedModes;

    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
  };
}
namespace winrt::Unpaint::factory_implementation
{
  struct ControlNetModelsViewModel : ControlNetModelsViewModelT<ControlNetModelsViewModel, implementation::ControlNetModelsViewModel>
  {
  };
}
