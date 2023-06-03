#pragma once
#include "ShowcaseViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ShowcaseViewModel : ShowcaseViewModelT<ShowcaseViewModel>
  {
    ShowcaseViewModel();
    ~ShowcaseViewModel();

    Windows::Foundation::Collections::IObservableVector<hstring> Images();    
    int32_t ImageCount();

    int32_t SelectedImageIndex();
    fire_and_forget SelectedImageIndex(int32_t value);

    hstring Prompt();

    void Continue();

    winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(winrt::event_token const& token) noexcept;

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    INavigationService _navigationService;
    Windows::Foundation::Collections::IObservableVector<hstring> _images;
    int32_t _selectedImageIndex;
    hstring _prompt;

    Windows::UI::Xaml::DispatcherTimer _timer;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ShowcaseViewModel : ShowcaseViewModelT<ShowcaseViewModel, implementation::ShowcaseViewModel>
  {
  };
}
