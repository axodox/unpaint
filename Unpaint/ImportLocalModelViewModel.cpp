#include "pch.h"
#include "ImportLocalModelViewModel.h"
#include "ImportLocalModelViewModel.g.cpp"
#include "Storage/UwpStorage.h"

using namespace Axodox::Storage;
using namespace Axodox::Web;
using namespace std;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  hstring ImportLocalModelViewModel::ModelPath()
  {
    return _modelFolder ? _modelFolder.Path() : L"Click on browse to select a model.";
  }

  bool ImportLocalModelViewModel::IsValid()
  {
    return _isValid;
  }

  hstring ImportLocalModelViewModel::Status()
  {
    return _status;
  }

  Windows::Storage::StorageFolder ImportLocalModelViewModel::Result()
  {
    return _modelFolder;
  }

  winrt::event_token ImportLocalModelViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ImportLocalModelViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }

  fire_and_forget ImportLocalModelViewModel::BrowseModelAsync()
  {
    FolderPicker folderPicker{};
    _modelFolder = co_await folderPicker.PickSingleFolderAsync();
        
    if (_modelFolder)
    {
      vector<StorageFile> files;
      co_await read_files_recursively(_modelFolder, files);

      auto count = 0;
      const auto& fileset = HuggingFaceModelDetails::StableDiffusionOnnxFileset;
      for (const auto& file : files)
      {
        auto fileName = to_string(file.Path()).substr(_modelFolder.Path().size() + 1);
        replace(fileName.begin(), fileName.end(), '\\', '/');
        if (fileset.contains(fileName)) count++;
      }

      _isValid = count == fileset.size();
      _status = _isValid ? L"" : L"The model does not match the Stable Diffusion ONNX schema.";
    }
    else
    {
      _isValid = false;
      _status = L"";
    }
    
    _propertyChanged(*this, PropertyChangedEventArgs(L"ModelPath"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsValid"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"Status"));
  }
}
