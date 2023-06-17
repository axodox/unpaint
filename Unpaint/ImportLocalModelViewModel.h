#pragma once
#include "ImportLocalModelViewModel.g.h"

namespace winrt::Unpaint::implementation
{
  struct ImportLocalModelViewModel : ImportLocalModelViewModelT<ImportLocalModelViewModel>
  {
    ImportLocalModelViewModel() = default;

    hstring ModelPath();
    bool IsValid();
    hstring Status();

    Windows::Storage::StorageFolder Result();

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(event_token const& token) noexcept;

    fire_and_forget BrowseModelAsync();

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;

    Windows::Storage::StorageFolder _modelFolder{nullptr};
    bool _isValid = false;
    hstring _status;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct ImportLocalModelViewModel : ImportLocalModelViewModelT<ImportLocalModelViewModel, implementation::ImportLocalModelViewModel>
  {
  };
}
