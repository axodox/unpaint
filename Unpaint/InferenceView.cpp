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
    _isPointerOverStatusBar(false),
    _isInputPaneVisible(false)
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

    //Configure view model
    _viewModelPropertyChangedRevoker = _viewModel.PropertyChanged(auto_revoke, { this, &InferenceView::OnViewModelPropertyChanged });
  }

  InferenceViewModel InferenceView::ViewModel()
  {
    return _viewModel;
  }

  bool InferenceView::IsStatusVisible()
  {
    return !_navigationService.IsPointerOverTitleBar() && !_viewModel.Status().empty();
  }

  void InferenceView::ToggleSettingsLock()
  {
    _viewModel.IsSettingsLocked(!_viewModel.IsSettingsLocked());
  }

  void InferenceView::ToggleJumpingToLatestImage()
  {
    _viewModel.IsJumpingToLatestImage(!_viewModel.IsJumpingToLatestImage());
  }

  bool InferenceView::IsInputPaneVisible()
  {
    return _isInputPaneVisible;
  }

  void InferenceView::ToggleInputPane()
  {
    _isInputPaneVisible = !_isInputPaneVisible;
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsInputPaneVisible"));

    const auto& viewModel = _viewModel;
    if (_isInputPaneVisible && !viewModel.InputImage())
    {
      viewModel.InputImage(viewModel.OutputImage());
    }
    else
    {
      viewModel.InputImage(nullptr);
    }
  }

  void InferenceView::OnOutputImageDragStarting(Windows::UI::Xaml::UIElement const& /*sender*/, Windows::UI::Xaml::DragStartingEventArgs const& eventArgs)
  {
    auto outputImage = _viewModel.OutputImage();
    if (!outputImage) return;

    eventArgs.AllowedOperations(DataPackageOperation::Copy);
    eventArgs.Data().SetStorageItems({ outputImage });

    OutputImagesView().AllowDrop(false);
  }

  void InferenceView::OnOutputImageDropCompleted(Windows::UI::Xaml::UIElement const& /*sender*/, Windows::UI::Xaml::DropCompletedEventArgs const& /*eventArgs*/)
  {
    OutputImagesView().AllowDrop(true);
  }

  void InferenceView::OnImageDragOver(Windows::Foundation::IInspectable const& /*sender*/, Windows::UI::Xaml::DragEventArgs const& eventArgs)
  {
    if (!eventArgs.DataView().Contains(StandardDataFormats::StorageItems())) return;

    eventArgs.AcceptedOperation(DataPackageOperation::Copy);
  }

  fire_and_forget InferenceView::OnImageDrop(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::DragEventArgs const& eventArgs)
  {
    const auto& dataView = eventArgs.DataView();
    if (!dataView.Contains(StandardDataFormats::StorageItems())) co_return;

    auto isOutput = sender == OutputImagesView();
    auto items = co_await dataView.GetStorageItemsAsync();
    for (const auto& item : items)
    {
      auto file = item.try_as<StorageFile>();
      if (!file) continue;

      if (isOutput)
      {
        _viewModel.AddImage(file);
      }
      else
      {
        _viewModel.InputImage(file);
      }
    }
  }

  void InferenceView::OnFirstImageInvoked(Windows::UI::Xaml::Input::KeyboardAccelerator const& /*sender*/, Windows::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& /*args*/)
  {
    if (_viewModel.Images().Size() > 0)
    {
      _viewModel.SelectedImageIndex(0);
    }
  }

  void InferenceView::OnPreviousImageInvoked(Windows::UI::Xaml::Input::KeyboardAccelerator const& /*sender*/, Windows::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& /*args*/)
  {
    auto currentIndex = _viewModel.SelectedImageIndex();
    if (currentIndex > 0)
    {
      _viewModel.SelectedImageIndex(currentIndex - 1);
    }
  }

  void InferenceView::OnNextImageInvoked(Windows::UI::Xaml::Input::KeyboardAccelerator const& /*sender*/, Windows::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& /*args*/)
  {
    auto currentIndex = _viewModel.SelectedImageIndex();
    if (currentIndex + 1 < int32_t(_viewModel.Images().Size()))
    {
      _viewModel.SelectedImageIndex(currentIndex + 1);
    }
  }

  void InferenceView::OnLastImageInvoked(Windows::UI::Xaml::Input::KeyboardAccelerator const& /*sender*/, Windows::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& /*args*/)
  {
    _viewModel.SelectedImageIndex(int32_t(_viewModel.Images().Size()) - 1);
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

  void InferenceView::OnViewModelPropertyChanged(Windows::Foundation::IInspectable const& /*sender*/, Windows::UI::Xaml::Data::PropertyChangedEventArgs const& eventArgs)
  {
    UpdateStatusVisibility();

    if (eventArgs.PropertyName() == L"SelectedModeIndex" && _viewModel.SelectedModeIndex() == 0 && IsInputPaneVisible())
    {
      ToggleInputPane();
    }

    if (eventArgs.PropertyName() == L"InputImage")
    {
      InputMaskEditor().ClearMask();
    }
  }
}