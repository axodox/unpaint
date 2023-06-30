#pragma once
#include "ControlNetOptionsViewModel.g.h"
#include "UnpaintState.h"

namespace winrt::Unpaint::implementation
{
  struct ControlNetOptionsViewModel : ControlNetOptionsViewModelT<ControlNetOptionsViewModel>
  {
    ControlNetOptionsViewModel();

    bool IsAvailable();
    Windows::Foundation::Collections::IObservableVector<Unpaint::ControlNetModeViewModel> Modes();

    bool IsEnabled();
    void IsEnabled(bool value);

    int32_t SelectedModeIndex();
    void SelectedModeIndex(int32_t value);

    void InstallModes();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(event_token const& token) noexcept;

  private:
    INavigationService _navigationService;
    std::shared_ptr<UnpaintState> _unpaintState;
    Windows::Foundation::Collections::IObservableVector<ControlNetModeViewModel> _modes;
    int32_t _selectedModeIndex = -1;
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
  };
}
namespace winrt::Unpaint::factory_implementation
{
  struct ControlNetOptionsViewModel : ControlNetOptionsViewModelT<ControlNetOptionsViewModel, implementation::ControlNetOptionsViewModel>
  {
  };
}
