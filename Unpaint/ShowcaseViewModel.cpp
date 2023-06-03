#include "pch.h"
#include "ShowcaseViewModel.h"
#include "ShowcaseViewModel.g.cpp"
#include "Infrastructure/WinRtDependencies.h"
#include "Storage/FileIO.h"
#include "Graphics/Textures/TextureData.h"
#include "ImageMetadata.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Axodox::Storage;
using namespace std;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  ShowcaseViewModel::ShowcaseViewModel() :
    _images(single_threaded_observable_vector<hstring>()),
    _navigationService(dependencies.resolve<INavigationService>())
  {
    //Load showcase image list
    for (auto& item : filesystem::directory_iterator{ app_folder() / "Showcase" })
    {
      if (item.is_directory() || item.path().extension() != ".png") continue;

      _images.Append(hstring(item.path().c_str()));
    }

    //Launch paging timer
    _timer.Tick([=](auto&, auto&) {
      _selectedImageIndex++;
      if (_selectedImageIndex >= int32_t(_images.Size())) _selectedImageIndex = 0;
      _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedImageIndex"));
      });
    _timer.Interval(5s);
    _timer.Start();

    //Load prompt
    SelectedImageIndex(0);
  }

  ShowcaseViewModel::~ShowcaseViewModel()
  {
    _timer.Stop();
  }

  Windows::Foundation::Collections::IObservableVector<hstring> ShowcaseViewModel::Images()
  {
    return _images;
  }

  int32_t ShowcaseViewModel::ImageCount()
  {
    return _images.Size();
  }

  int32_t ShowcaseViewModel::SelectedImageIndex()
  {
    return _selectedImageIndex;
  }

  fire_and_forget ShowcaseViewModel::SelectedImageIndex(int32_t value)
  {
    _selectedImageIndex = value;
    _propertyChanged(*this, PropertyChangedEventArgs(L"SelectedImageIndex"));

    apartment_context callerContext;

    auto lifetime = get_strong();
    co_await resume_background();
    
    auto imageBuffer = try_read_file(to_string(_images.GetAt(value)));

    string imageMetadata;
    TextureData::FromBuffer(imageBuffer, TextureImageFormat::Rgba8, &imageMetadata);
    auto prompt = to_hstring(*try_parse_json<ImageMetadata>(imageMetadata)->PositivePrompt);

    co_await callerContext;
    _prompt = prompt;
    _propertyChanged(*this, PropertyChangedEventArgs(L"Prompt"));
  }

  hstring ShowcaseViewModel::Prompt()
  {
    return _prompt;
  }

  void ShowcaseViewModel::Continue()
  {
    _navigationService.NavigateToView(xaml_typename<WelcomeView>());
  }

  winrt::event_token ShowcaseViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ShowcaseViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }
}
