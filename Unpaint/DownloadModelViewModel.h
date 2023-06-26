#pragma once
#include "DownloadModelViewModel.g.h"
#include "Threading/AsyncOperation.h"

namespace winrt::Unpaint::implementation
{
  struct DownloadModelViewModel : DownloadModelViewModelT<DownloadModelViewModel>
  {
    double Progress();
    hstring StatusMessage();

    bool IsInProgress();
    bool IsFinished();

    void Cancel();

    fire_and_forget DownloadStableDiffusionModelAsync(hstring modelId);

    event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
    void PropertyChanged(event_token const& token);

  private:
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    apartment_context _uiContext;
    Axodox::Threading::async_operation _downloadOperation;

    double _progress = 0.0;
    hstring _statusMessage;
    Axodox::Threading::async_operation_state _state = {};

    fire_and_forget OnDownloadStateChanged(Axodox::Threading::async_operation_info state);

  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct DownloadModelViewModel : DownloadModelViewModelT<DownloadModelViewModel, implementation::DownloadModelViewModel>
  {
  };
}
