#pragma once
#include "App.xaml.g.h"
#include "SizeToStringConverter.h"
#include "IsNanToBooleanConverter.h"
#include "NanToZeroConverter.h"
#include "StringToImageSourceConverter.h"
#include "BooleanInverter.h"
#include "EqualsConverter.h"
#include "BooleanToGridLengthConverter.h"
#include "StorageFileToImageSourceConverter.h"
#include "BooleanSwitchConverter.h"
#include "IsNullConverter.h"
#include "ModelRepository.h"
#include "DeviceInformation.h"
#include "UnpaintState.h"

namespace winrt::Unpaint::implementation
{
  struct App : AppT<App, INavigationService>
  {
    App();
    void Activate(Windows::ApplicationModel::Activation::IActivatedEventArgs eventArgs);

    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
    void OnActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs const& eventArgs);
    void OnSuspending(IInspectable const&, Windows::ApplicationModel::SuspendingEventArgs const&);
    void OnNavigationFailed(IInspectable const&, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);

    void NavigateToView(Windows::UI::Xaml::Interop::TypeName viewType);

    bool IsPointerOverTitleBar();

    event_token IsPointerOverTitleBarChanged(Windows::Foundation::EventHandler<bool> const& handler);
    void IsPointerOverTitleBarChanged(event_token const& token) noexcept;

  private:
    std::shared_ptr<UnpaintState> _unpaintState;
    std::shared_ptr<ModelRepository> _modelRepository;
    std::shared_ptr<DeviceInformation> _deviceInformation;
    Windows::UI::Xaml::Controls::Frame _frame;
    bool _isPointerOverTitleBar = false;

    event<Windows::Foundation::EventHandler<bool>> _isPointerOverTitleBarChanged;
  };
}
