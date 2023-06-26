#include "pch.h"
#include "DownloadModelViewModel.h"
#include "DownloadModelViewModel.g.cpp"
#include "Infrastructure/DependencyContainer.h"
#include "ModelRepository.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace winrt;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  double DownloadModelViewModel::Progress()
  {
    return _progress;
  }

  hstring DownloadModelViewModel::StatusMessage()
  {
    return _statusMessage;
  }

  bool DownloadModelViewModel::IsInProgress()
  {
    return _state == async_operation_state::working;
  }

  bool DownloadModelViewModel::IsFinished()
  {
    return _state != async_operation_state::idle && _state != async_operation_state::working;
  }

  void DownloadModelViewModel::Cancel()
  {
    _downloadOperation.cancel();
  }

  event_token DownloadModelViewModel::PropertyChanged(PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void DownloadModelViewModel::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }

  fire_and_forget DownloadModelViewModel::DownloadStableDiffusionModelAsync(hstring modelId)
  {
    //Switch to background thread
    auto lifetime = get_strong();
    co_await resume_background();

    //Resolve dependencies
    auto modelRepository = dependencies.resolve<ModelRepository>();

    //Run download
    _downloadOperation.state_changed(no_revoke, event_handler{ this, &DownloadModelViewModel::OnDownloadStateChanged });    
    modelRepository->TryInstallHuggingFaceModel(to_string(modelId), _downloadOperation);

    //Switch to foreground thread
    co_await _uiContext;
  }
  
  fire_and_forget DownloadModelViewModel::OnDownloadStateChanged(Axodox::Threading::async_operation_info state)
  {
    co_await _uiContext;

    _progress = isnan(state.progress) ? 0 : state.progress;
    _statusMessage = to_hstring(state.status_message);
    _state = state.state;

    _propertyChanged(*this, PropertyChangedEventArgs(L"Progress"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"StatusMessage"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsInProgress"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsFinished"));
  }
}
