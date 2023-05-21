#include "pch.h"
#include "InferenceView.h"
#include "InferenceView.g.cpp"
#include "Infrastructure/WinRtDependencies.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  InferenceView::InferenceView() :
    _navigationService(dependencies.resolve<INavigationService>()),
    _isPointerOverStatusBar(false)
  {
    InitializeComponent();

    //Configure title bar
    auto coreTitleBar = CoreApplication::GetCurrentView().TitleBar();
    _titleBarLayoutMetricsChangedRevoker = coreTitleBar.LayoutMetricsChanged(auto_revoke, [=](auto&, auto&) {
      StatusBar().Height(coreTitleBar.Height());
    });
    StatusBar().Height(coreTitleBar.Height());

    //Configure command panel
    _isPointerOverTitleBarChangedRevoker = _navigationService.IsPointerOverTitleBarChanged(auto_revoke, [=](auto&, auto&) {
      UpdateStatusVisibility();
      });
    _viewModelPropertyChangedRevoker = ViewModel().PropertyChanged(auto_revoke, [=](auto&, auto&) {
      UpdateStatusVisibility();
      });
  }

  InferenceViewModel InferenceView::ViewModel()
  {
    return _viewModel;
  }
  
  bool InferenceView::IsStatusVisible()
  {
    return !_navigationService.IsPointerOverTitleBar() && !ViewModel().Status().empty();
  }

  void InferenceView::ToggleSettingsLock()
  {
    _viewModel.IsSettingsLocked(!_viewModel.IsSettingsLocked());
  }

  void InferenceView::OnOutputImageDragStarting(Windows::UI::Xaml::UIElement const& /*sender*/, Windows::UI::Xaml::DragStartingEventArgs const& eventArgs)
  {
    eventArgs.AllowedOperations(DataPackageOperation::Copy);
    eventArgs.Data().SetStorageItems({ ViewModel().OutputImage() });
  }

  void InferenceView::OnOutputImageDragOver(Windows::Foundation::IInspectable const& /*sender*/, Windows::UI::Xaml::DragEventArgs const& eventArgs)
  {
    if (!eventArgs.DataView().Contains(StandardDataFormats::StorageItems())) return;

    eventArgs.AcceptedOperation(DataPackageOperation::Copy);
  }

  fire_and_forget InferenceView::OnOutputImageDrop(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::DragEventArgs const& eventArgs)
  {
    const auto& dataView = eventArgs.DataView();
    if (!dataView.Contains(StandardDataFormats::StorageItems())) co_return;

    auto items = co_await dataView.GetStorageItemsAsync();
    for (const auto& item : items)
    {
      auto file = item.try_as<StorageFile>();
      if (!file) continue;

      ViewModel().AddImage(file);
    }
  }

  event_token InferenceView::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value)
  {
    return _propertyChanged.add(value);
  }

  void InferenceView::PropertyChanged(event_token const& token)
  {
    _propertyChanged.remove(token);
  }
  
  void InferenceView::UpdateStatusVisibility()
  {
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsStatusVisible"));
  }
}
